#!/usr/bin/env python3
"""
通用的多方任务同步执行基础设施

用户只需提供：
1. 任务定义（命令列表）
2. 输出filter函数（可选）
3. log目录

Usage:
    from task_runner import TaskOrchestrator, Task
    
    # 定义任务
    tasks = [
        Task(
            name="experiment_1",
            commands=["./binary 0 -args", "./binary 1 -args", "./binary 2 -args"],
            output_filter=my_custom_filter  # 可选
        )
    ]
    
    # 运行
    orchestrator = TaskOrchestrator(
        log_dir="./logs",
        repeats=3
    )
    orchestrator.run(tasks)
"""

import subprocess
import shlex
import time
import json
import threading
import socket
from pathlib import Path
from datetime import datetime
from typing import Optional, List, Dict, Callable, Any
from dataclasses import dataclass, field
from collections import defaultdict


# ============================================================================
# 1. 同步机制基础设施
# ============================================================================

class BarrierSynchronizer:
    """
    多节点同步屏障机制
    确保所有参与方在继续前都完成了当前阶段
    """
    
    def __init__(self, host: str = "localhost", port: int = 0, num_participants: int = 1):
        """
        Args:
            host: 屏障服务器地址
            port: 屏障服务器端口（0表示不使用同步）
            num_participants: 参与方数量
        """
        self.host = host
        self.port = port
        self.num_participants = num_participants
        self._server = None
        self._enabled = port > 0
        
    def start_server(self):
        """启动屏障服务器（仅在主控节点上调用）"""
        if not self._enabled:
            return
        self._server = _BarrierServer(self.host, self.port, self.num_participants)
        self._server.start()
        
    def stop_server(self):
        """停止屏障服务器"""
        if self._server:
            self._server.stop()
            
    def wait(self, barrier_key: str, timeout: float = 300.0):
        """
        等待所有参与方到达屏障点
        
        Args:
            barrier_key: 屏障标识符
            timeout: 超时时间（秒）
        """
        if not self._enabled:
            return
        _barrier_wait(self.host, self.port, barrier_key, timeout)


class _BarrierServer(threading.Thread):
    """内部屏障服务器实现"""
    
    def __init__(self, host: str, port: int, num_participants: int):
        super().__init__(daemon=True)
        self.host = host
        self.port = port
        self.num_participants = num_participants
        self._stop_event = threading.Event()
        self._server_socket = None
        self._lock = threading.Lock()
        self._waiting = defaultdict(list)  # key -> list of sockets
        
    def stop(self):
        self._stop_event.set()
        if self._server_socket:
            try:
                self._server_socket.close()
            except Exception:
                pass
                
    def _handle_client(self, conn: socket.socket):
        try:
            data = conn.recv(1024)
            if not data:
                conn.close()
                return
            key = data.decode().strip()
            
            with self._lock:
                self._waiting[key].append(conn)
                if len(self._waiting[key]) >= self.num_participants:
                    # 释放所有等待的连接
                    sockets = self._waiting[key]
                    for s in sockets:
                        try:
                            s.sendall(b"GO\n")
                            s.close()
                        except Exception:
                            pass
                    del self._waiting[key]
        except Exception:
            try:
                conn.close()
            except Exception:
                pass
                
    def run(self):
        srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        srv.bind((self.host, self.port))
        srv.listen(64)
        self._server_socket = srv
        print(f"[SYNC] Barrier server listening on {self.host}:{self.port}")
        
        try:
            while not self._stop_event.is_set():
                try:
                    srv.settimeout(1.0)
                    conn, _ = srv.accept()
                    threading.Thread(target=self._handle_client, args=(conn,), daemon=True).start()
                except socket.timeout:
                    continue
        finally:
            try:
                srv.close()
            except Exception:
                pass


def _barrier_wait(host: str, port: int, key: str, timeout: float):
    """等待屏障"""
    start = time.time()
    while True:
        try:
            with socket.create_connection((host, port), timeout=5.0) as s:
                s.sendall((key + "\n").encode())
                s.settimeout(max(1.0, timeout - (time.time() - start)))
                data = s.recv(16)
                if data and data.strip() == b"GO":
                    return
        except (OSError, socket.timeout):
            pass
        if time.time() - start > timeout:
            print(f"[SYNC] Timeout waiting for barrier '{key}'")
            return
        time.sleep(0.2)


# ============================================================================
# 2. 任务定义
# ============================================================================

@dataclass
class Task:
    """
    任务定义
    
    Attributes:
        name: 任务名称
        commands: 要执行的命令列表（每个命令对应一个参与方）
        output_filter: 可选的输出过滤函数，从日志中提取数据
        params: 可选的参数字典，用于填充命令模板
    """
    name: str
    commands: List[str]
    output_filter: Optional[Callable[[str], Dict[str, Any]]] = None
    params: Dict[str, Any] = field(default_factory=dict)


# ============================================================================
# 3. 任务执行器
# ============================================================================

class TaskExecutor:
    """执行单个任务的多个命令，管理进程和日志"""
    
    def __init__(self, log_root: Path, save_metadata: bool = False):
        self.log_root = log_root
        self.save_metadata = save_metadata
        
    def execute(
        self,
        task: Task,
        run_index: int,
        synchronizer: Optional[BarrierSynchronizer] = None,
        command_filter: Optional[Callable[[int, str], bool]] = None
    ) -> Dict[str, Any]:
        """
        执行任务的一次运行
        
        Args:
            task: 任务定义
            run_index: 运行序号
            synchronizer: 可选的同步器
            command_filter: 可选的命令过滤函数（party_id, cmd）-> bool
            
        Returns:
            包含运行结果的字典
        """
        run_dir = self.log_root / task.name / f"run_{run_index:02d}"
        run_dir.mkdir(parents=True, exist_ok=True)
        
        # 填充命令模板
        filled_commands = [cmd.format(**task.params) for cmd in task.commands]
        
        # 应用命令过滤器
        if command_filter:
            command_list = [
                (i, cmd) for i, cmd in enumerate(filled_commands)
                if command_filter(i, cmd)
            ]
        else:
            command_list = list(enumerate(filled_commands))
            
        print(f"\n[TASK] {task.name} | Run {run_index}")
        for party_id, cmd in command_list:
            print(f"  Party {party_id}: {cmd}")
            
        # 同步：开始屏障
        if synchronizer:
            barrier_key = f"{task.name}:run-{run_index:02d}:start"
            print(f"[SYNC] Waiting at start barrier: {barrier_key}")
            synchronizer.wait(barrier_key)
            
        # 启动所有进程
        processes = []
        log_files = []
        start_time = time.time()
        
        for party_id, cmd in command_list:
            log_path = run_dir / f"party_{party_id}.log"
            proc, f = self._launch_process(cmd, log_path)
            processes.append((party_id, proc))
            log_files.append(f)
            
        # 等待所有进程完成
        returncodes = {}
        for party_id, proc in processes:
            proc.wait()
            returncodes[party_id] = proc.returncode
            
        # 关闭日志文件
        for f in log_files:
            try:
                f.flush()
                f.close()
            except Exception:
                pass
                
        end_time = time.time()
        elapsed = end_time - start_time
        
        print(f"[TASK] {task.name} | Run {run_index} completed in {elapsed:.2f}s")
        
        # 同步：结束屏障
        if synchronizer:
            barrier_key = f"{task.name}:run-{run_index:02d}:end"
            print(f"[SYNC] Waiting at end barrier: {barrier_key}")
            synchronizer.wait(barrier_key)
            
        # 解析日志
        metrics = {}
        if task.output_filter:
            for party_id, _ in command_list:
                log_path = run_dir / f"party_{party_id}.log"
                if log_path.exists():
                    try:
                        log_content = log_path.read_text(errors="ignore")
                        party_metrics = task.output_filter(log_content)
                        metrics[party_id] = party_metrics
                    except Exception as e:
                        print(f"[WARN] Failed to parse log for party {party_id}: {e}")
                        
        # 返回运行结果
        result = {
            "run_index": run_index,
            "start_time": start_time,
            "end_time": end_time,
            "elapsed_sec": elapsed,
            "returncodes": returncodes,
            "metrics": metrics
        }
        
        # 保存运行元数据（可选）
        if self.save_metadata:
            with open(run_dir / "run_meta.json", "w") as f:
                json.dump(result, f, indent=2)
            
        return result
        
    def _launch_process(self, cmd: str, log_path: Path):
        """启动单个进程"""
        args = shlex.split(cmd)
        log_path.parent.mkdir(parents=True, exist_ok=True)
        f = open(log_path, "wb")
        proc = subprocess.Popen(args, stdout=f, stderr=subprocess.STDOUT)
        return proc, f


# ============================================================================
# 4. 主编排器
# ============================================================================

class TaskOrchestrator:
    """
    任务编排器 - 简单的接口
    
    用户只需提供任务列表、log目录、重复次数等基本参数
    """
    
    def __init__(
        self,
        log_dir: str = "./logs",
        repeats: int = 3,
        sync_host: Optional[str] = None,
        sync_port: int = 0,
        num_participants: int = 1,
        serve_barrier: bool = False,
        command_filter: Optional[Callable[[int, str], bool]] = None,
        save_metadata: bool = False
    ):
        """
        Args:
            log_dir: 日志目录（默认：./logs/{timestamp}）
            repeats: 每个任务重复次数
            sync_host: 同步服务器地址（None表示不使用同步）
            sync_port: 同步服务器端口（0表示不使用同步）
            num_participants: 参与方数量
            serve_barrier: 是否启动屏障服务器
            command_filter: 命令过滤函数，用于选择性运行命令
            save_metadata: 是否保存元数据JSON文件（默认：False）
        """
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        self.log_root = Path(log_dir) / timestamp if "{timestamp}" in log_dir or log_dir == "./logs" else Path(log_dir)
        self.log_root.mkdir(parents=True, exist_ok=True)
        self.repeats = repeats
        self.command_filter = command_filter
        self.save_metadata = save_metadata
        
        # 设置同步器
        if sync_host and sync_port > 0:
            self.synchronizer = BarrierSynchronizer(sync_host, sync_port, num_participants)
            if serve_barrier:
                self.synchronizer.start_server()
        else:
            self.synchronizer = None
            
        self.executor = TaskExecutor(self.log_root, save_metadata)
        
    def run(self, tasks: List[Task]) -> Dict[str, Any]:
        """
        运行所有任务
        
        Args:
            tasks: 任务列表
            
        Returns:
            包含所有任务结果的汇总字典
        """
        print(f"[ORCHESTRATOR] Starting {len(tasks)} tasks, {self.repeats} runs each")
        print(f"[ORCHESTRATOR] Logs will be saved to: {self.log_root.resolve()}")
        
        all_results = []
        overall_start = time.time()
        
        try:
            for task in tasks:
                task_results = self._run_task(task)
                all_results.append(task_results)
                time.sleep(1.0)  # 任务间暂停
        finally:
            if self.synchronizer:
                self.synchronizer.stop_server()
                
        overall_end = time.time()
        
        # 保存总体汇总
        summary = {
            "started_at": datetime.fromtimestamp(overall_start).strftime("%Y%m%d_%H%M%S"),
            "ended_at": datetime.fromtimestamp(overall_end).strftime("%Y%m%d_%H%M%S"),
            "elapsed_sec": overall_end - overall_start,
            "tasks": all_results
        }
        
        if self.save_metadata:
            with open(self.log_root / "summary.json", "w") as f:
                json.dump(summary, f, indent=2)
            
        print(f"\n[ORCHESTRATOR] All tasks completed in {summary['elapsed_sec']:.2f}s")
        print(f"[ORCHESTRATOR] Results saved to: {self.log_root.resolve()}")
        
        return summary
        
    def _run_task(self, task: Task) -> Dict[str, Any]:
        """运行单个任务的所有重复"""
        print(f"\n{'='*60}")
        print(f"[TASK] Starting: {task.name}")
        print(f"{'='*60}")
        
        runs = []
        metrics_aggregator = defaultdict(lambda: defaultdict(list))
        
        for i in range(1, self.repeats + 1):
            result = self.executor.execute(
                task,
                i,
                self.synchronizer,
                self.command_filter
            )
            runs.append(result)
            
            # 聚合metrics
            for party_id, party_metrics in result.get("metrics", {}).items():
                for key, value in party_metrics.items():
                    if isinstance(value, (int, float)):
                        metrics_aggregator[party_id][key].append(float(value))
                        
            time.sleep(0.5)  # 运行间暂停
            
        # 计算平均值
        averages = {}
        for party_id, metrics in metrics_aggregator.items():
            party_avg = {}
            for key, values in metrics.items():
                if values:
                    party_avg[key] = sum(values) / len(values)
            if party_avg:
                averages[str(party_id)] = party_avg
                
        task_summary = {
            "task_name": task.name,
            "runs": runs,
            "averages": averages
        }
        
        # 保存任务汇总（可选）
        if self.save_metadata:
            task_dir = self.log_root / task.name
            with open(task_dir / "task_summary.json", "w") as f:
                json.dump(task_summary, f, indent=2)
            
        return task_summary


# ============================================================================
# 5. 内置输出过滤器
# ============================================================================

class BuiltinFilters:
    """内置的常用输出过滤器"""
    
    @staticmethod
    def regex_filter(patterns: Dict[str, str]) -> Callable[[str], Dict[str, Any]]:
        """
        创建基于正则表达式的过滤器
        
        Args:
            patterns: 字典 {metric_name: regex_pattern}
                     regex必须包含一个捕获组来提取数值
                     
        Returns:
            过滤器函数
            
        Example:
            filter_fn = BuiltinFilters.regex_filter({
                "compute_time": r"Compute.*takes\s+([0-9.]+)\s+seconds",
                "cpu_time": r"CPU time:\s+([0-9.]+)\s+seconds"
            })
        """
        import re
        
        def filter_fn(log_content: str) -> Dict[str, Any]:
            metrics = {}
            for name, pattern in patterns.items():
                match = re.search(pattern, log_content, re.IGNORECASE)
                if match:
                    try:
                        metrics[name] = float(match.group(1))
                    except (ValueError, IndexError):
                        pass
            return metrics
            
        return filter_fn
        
    @staticmethod
    def key_value_filter(separator: str = ":", prefix: str = "") -> Callable[[str], Dict[str, Any]]:
        """
        创建键值对过滤器
        
        Args:
            separator: 键值分隔符
            prefix: 可选的行前缀过滤
            
        Example:
            # 解析形如 "METRIC: value" 的行
            filter_fn = BuiltinFilters.key_value_filter(":", "METRIC")
        """
        def filter_fn(log_content: str) -> Dict[str, Any]:
            metrics = {}
            for line in log_content.splitlines():
                if prefix and not line.startswith(prefix):
                    continue
                if separator in line:
                    key, _, value = line.partition(separator)
                    key = key.strip()
                    value = value.strip()
                    try:
                        metrics[key] = float(value)
                    except ValueError:
                        metrics[key] = value
            return metrics
            
        return filter_fn
