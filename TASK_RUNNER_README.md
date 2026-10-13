# Task Runner - 通用任务执行基础设施

这是一个简化和模块化的任务执行框架，用于运行需要多方同步的任务（如MPC benchmarks）。

## 核心特性

✅ **简单** - 只需提供命令、过滤器、日志目录
✅ **模块化** - 同步机制、任务执行、日志解析独立模块
✅ **通用** - 适用于任何需要多方协同的任务
✅ **可扩展** - 自定义输出过滤器提取你需要的数据

## 架构

```
task_runner.py
├── BarrierSynchronizer    # 多节点同步机制
├── TaskExecutor           # 任务执行引擎
├── TaskOrchestrator       # 主编排器
├── Task                   # 任务定义
└── BuiltinFilters         # 内置过滤器
```

## 快速开始

### 1. 基本用法

```python
from task_runner import TaskOrchestrator, Task

# 定义任务
tasks = [
    Task(
        name="my_task",
        commands=[
            "./program 0 --args",
            "./program 1 --args",
            "./program 2 --args",
        ]
    )
]

# 运行
orchestrator = TaskOrchestrator(log_dir="./logs", repeats=3)
orchestrator.run(tasks)
```

### 2. 使用输出过滤器

```python
from task_runner import TaskOrchestrator, Task, BuiltinFilters

# 使用正则表达式过滤器提取数据
my_filter = BuiltinFilters.regex_filter({
    "compute_time": r"Compute takes\s+([0-9.]+)\s+seconds",
    "throughput": r"Throughput:\s+([0-9.]+)\s+ops/sec",
})

tasks = [
    Task(
        name="benchmark",
        commands=["./benchmark 0", "./benchmark 1", "./benchmark 2"],
        output_filter=my_filter,  # 从日志中自动提取metrics
    )
]

orchestrator = TaskOrchestrator(log_dir="./logs", repeats=3)
orchestrator.run(tasks)
```

### 3. 参数化命令

```python
tasks = [
    Task(
        name="parameterized",
        commands=[
            "./bench 0 -n {num_gates} -h {host}",
            "./bench 1 -n {num_gates} -h {host}",
            "./bench 2 -n {num_gates} -h {host}",
        ],
        params={"num_gates": 1000000, "host": "localhost"},
    )
]
```

### 4. 多节点同步

**配置方式：直接修改代码中的 SYNC_CONFIG**

在 `run_benchmarks.py` 中修改配置：

```python
# 主节点配置（运行party 0）
SYNC_CONFIG = {
    "enabled": True,
    "serve_barrier": True,          # 主节点启动服务器
    "sync_host": "192.168.1.100",   # 主节点IP
    "sync_port": 9999,
    "num_nodes": 3,
    "party_ids": [0],               # 只运行party 0
}
```

```python
# 工作节点配置（运行party 1）
SYNC_CONFIG = {
    "enabled": True,
    "serve_barrier": False,         # 工作节点不启动服务器
    "sync_host": "192.168.1.100",   # 连接到主节点
    "sync_port": 9999,
    "num_nodes": 3,
    "party_ids": [1],               # 只运行party 1
}
```

然后在各节点上运行：
```bash
python3 run_benchmarks.py
```

## 自定义过滤器

### 内置过滤器

#### 1. 正则表达式过滤器
```python
filter_fn = BuiltinFilters.regex_filter({
    "metric_name": r"Pattern with (\d+\.?\d*) capture group",
})
```

#### 2. 键值对过滤器
```python
# 解析 "Key: Value" 格式
filter_fn = BuiltinFilters.key_value_filter(separator=":", prefix="METRIC")
```

### 自定义过滤器

```python
def my_custom_filter(log_content: str) -> dict:
    """
    自定义过滤器函数
    
    Args:
        log_content: 日志文件的完整内容
        
    Returns:
        metrics字典，包含从日志中提取的数据
    """
    metrics = {}
    
    # 你的解析逻辑
    for line in log_content.splitlines():
        if "Time:" in line:
            # 提取时间
            time_str = line.split(":")[1].strip()
            metrics["execution_time"] = float(time_str)
    
    return metrics

# 使用自定义过滤器
task = Task(
    name="my_task",
    commands=["./program 0"],
    output_filter=my_custom_filter,
)
```

## 日志输出结构

```
logs/
└── 20260205_123456/              # 时间戳
    ├── summary.json               # 总体汇总
    ├── run_config.json            # 运行配置
    ├── task_name_1/               # 任务1
    │   ├── task_summary.json      # 任务汇总（包含平均值）
    │   ├── run_01/                # 第1次运行
    │   │   ├── party_0.log        # Party 0的输出
    │   │   ├── party_1.log        # Party 1的输出
    │   │   ├── party_2.log        # Party 2的输出
    │   │   └── run_meta.json      # 运行元数据
    │   ├── run_02/                # 第2次运行
    │   └── run_03/                # 第3次运行
    └── task_name_2/               # 任务2
        └── ...
```

### JSON输出示例

**task_summary.json:**
```json
{
  "task_name": "semi_mul",
  "runs": [...],
  "averages": {
    "0": {
      "compute_time": 12.34,
      "cpu_time": 11.50
    },
    "1": {
      "compute_time": 12.40,
      "cpu_time": 11.55
    }
  }
}
```

## 使用MPC Benchmarks

### 运行所有benchmarks
```bash
python3 run_benchmarks.py --repeats 3
```

### 运行特定benchmarks
```bash
python3 run_benchmarks.py semi ccs --repeats 5
```

### 自定义参数

直接修改代码中的 `PARAMS` 配置：
```python
PARAMS = {
    "round": 20,
    "num_gates": 2097152,
    "batch_size": 40000000,
    "binary_batch_size": 10485760,
    "host": "localhost",
    "port": 12345,
}
```

### 查看可用任务
```bash
python3 run_benchmarks.py --help
```

## API 参考

### TaskOrchestrator

主编排器，管理任务的执行。

```python
TaskOrchestrator(
    log_dir: str = "./logs",              # 日志目录
    repeats: int = 3,                     # 每个任务重复次数
    sync_host: Optional[str] = None,      # 同步服务器地址
    sync_port: int = 0,                   # 同步服务器端口
    num_participants: int = 1,            # 参与方数量
    serve_barrier: bool = False,          # 是否启动屏障服务器
    command_filter: Optional[Callable] = None  # 命令过滤函数
)
```

**方法:**
- `run(tasks: List[Task]) -> Dict[str, Any]` - 运行任务列表

### Task

任务定义。

```python
Task(
    name: str,                           # 任务名称
    commands: List[str],                 # 命令列表
    output_filter: Optional[Callable] = None,  # 输出过滤器
    params: Dict[str, Any] = {}          # 参数字典
)
```

### BarrierSynchronizer

多节点同步机制。

```python
BarrierSynchronizer(
    host: str = "localhost",
    port: int = 0,                       # 0表示不使用同步
    num_participants: int = 1
)
```

**方法:**
- `start_server()` - 启动屏障服务器
- `stop_server()` - 停止屏障服务器
- `wait(barrier_key: str, timeout: float)` - 等待屏障

## 示例代码

查看以下文件获取更多示例：

- `example_custom_tasks.py` - 各种使用场景的示例
- `run_benchmarks.py` - MPC benchmarks的完整实现

## 对比原始版本

### 原始版本
- 711行代码，功能耦合
- 同步机制、任务执行、配置混在一起
- 难以扩展到其他用途
- 需要深入理解代码才能修改

### 新版本 (task_runner.py + run_benchmarks.py)
- **核心基础设施** (`task_runner.py`)：~450行，通用可复用
- **应用代码** (`run_benchmarks.py`)：~370行，配置清晰
- **清晰分离**：同步机制、任务执行、日志解析独立模块
- **易于使用**：配置直接写在代码顶部，一目了然
- **通用基础设施**：可用于任何需要多方协同的任务
- **易于扩展**：添加新实验只需添加 Task 定义

## 进阶用法

### 1. 动态任务生成

```python
# 为不同参数生成多个任务
batch_sizes = [1000000, 2000000, 5000000]
tasks = []

for bs in batch_sizes:
    tasks.append(Task(
        name=f"benchmark_bs_{bs}",
        commands=[f"./bench 0 -b {bs}", f"./bench 1 -b {bs}"],
    ))

orchestrator.run(tasks)
```

### 2. 条件执行

```python
# 只在某些节点上运行特定命令
def my_party_filter(party_id: int, cmd: str) -> bool:
    # 只运行party 0和2
    return party_id in [0, 2]

orchestrator = TaskOrchestrator(
    command_filter=my_party_filter
)
```

### 3. 组合过滤器

```python
def combined_filter(log_content: str) -> dict:
    # 先用正则表达式提取
    metrics = BuiltinFilters.regex_filter({...})(log_content)
    
    # 再添加自定义逻辑
    if "ERROR" in log_content:
        metrics["has_error"] = True
    
    return metrics
```

## 故障排除

### 问题：屏障超时
**解决：** 检查所有节点是否正确连接到同步服务器，确认`--num-nodes`参数正确。

### 问题：命令未找到
**解决：** 确保二进制文件存在且有执行权限，或使用绝对路径。

### 问题：日志未包含预期数据
**解决：** 检查过滤器的正则表达式是否正确，使用`errors="ignore"`处理编码问题。

## 贡献

欢迎提交改进建议！主要改进方向：
- 更多内置过滤器
- 更强大的同步机制
- 实时进度显示
- 更详细的错误报告

## License

与Covert-3PC项目相同的许可证。
