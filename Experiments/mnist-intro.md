# MNIST Networks Overview

## 网络架构对比

这四个程序实现了 SecureNN 论文中的不同 MNIST 网络架构：

| 网络 | 架构类型 | 结构描述 | 复杂度 | 特点 |
|------|---------|---------|--------|------|
| **A** | 全连接网络 | 3层Dense (784→128→128→10) | ⭐ | 最简单，纯全连接，支持丰富配置选项 |
| **B** | 中等卷积网络 | 2×Conv2d(16 filters) + 2×Dense | ⭐⭐⭐ | 平衡性能与复杂度 |
| **C** | LeNet变体 | 2×Conv2d(20/50 filters) + 2×Dense | ⭐⭐⭐⭐ | 最强性能，经典LeNet架构 |
| **D** | 轻量卷积网络 | 1×Conv2d(5 filters) + 可配置Dense | ⭐⭐ | 最灵活，需命令行参数配置 |

### 详细结构

#### mnist_full_A (全连接)
```
Input (28×28=784)
  ↓
Dense(784 → 128) + ReLU
  ↓
Dense(128 → 128) + ReLU
  ↓
Dense(128 → 10)
  ↓
Output (10 classes)
```

#### mnist_full_B (中等卷积)
```
Input (28×28×1)
  ↓
Conv2d(5×5, 16 filters) → MaxPool(2×2) → ReLU (12×12×16)
  ↓
Conv2d(5×5, 16 filters) → MaxPool(2×2) → ReLU (4×4×16)
  ↓
Flatten → Dense(256 → 100) + ReLU
  ↓
Dense(100 → 10)
  ↓
Output (10 classes)
```

#### mnist_full_C (LeNet)
```
Input (28×28×1)
  ↓
Conv2d(5×5, 20 filters) → MaxPool(2×2) → ReLU (12×12×20)
  ↓
Conv2d(5×5, 50 filters) → MaxPool(2×2) → ReLU (4×4×50)
  ↓
Flatten → Dense(800 → 500) + ReLU
  ↓
Dense(500 → 10)
  ↓
Output (10 classes)
```

#### mnist_full_D (轻量可配置)
```
Input (28×28×1)
  ↓
Conv2d(5×5, 5 filters, stride可配置) → ReLU → [可选MaxPool]
  ↓
[根据参数选择1dense或2dense]
  ↓
Output (10 classes)
```

---

## 命令行参数说明

### 通用位置参数（所有网络）

| 位置 | 参数名 | 默认值 | 说明 |
|------|--------|--------|------|
| args[1] | `n_epochs` | 100 | 训练轮数（epochs） |
| args[2] | `batch_size` | 见下表 | 批次大小 |
| args[3] | `n_threads` | auto | 线程数 |

**batch_size 默认值：**
- A: N (全部样本)
- B/C/D: min(N, 128)

### 通用标志参数（所有网络）

| 参数 | 作用 | 数据集大小 |
|------|------|-----------|
| `profile` | 性能分析模式 | N=1000, test=100 |
| `debug` | 调试模式 | N=100, test=100 |
| `debug1000` | 调试模式（B/C/D） | N=1000, test=1000 |
| `debug5000` | 调试模式 | N=5000, test=5000 |
| `mini` | 最小模式（仅A） | N=10, test=10 |
| `no_acc` | 跳过准确率计算 | - |
| `no_loss` | 跳过损失计算 | - |

默认（无标志）：N=60000, test=10000

---

## 网络特定参数

### mnist_full_A 专属参数

#### 网络结构参数
| 参数 | 说明 |
|------|------|
| `fc512` | 将隐藏层神经元数从128改为512 |
| `1dense` | 只使用1个Dense层 (784→10) |
| `2dense` | 只使用2个Dense层 (删除第二个隐藏层) |
| `norelu` | 不使用ReLU激活函数（使用恒等函数） |

#### 正则化参数
| 参数 | 说明 |
|------|------|
| `batchnorm` | 在第一个隐藏层后添加批归一化 |
| `dropout` | 在所有隐藏层后添加Dropout |
| `dropout-late` | 仅在最后一个隐藏层后添加Dropout |
| `dropout-early` | 仅在输入层后添加Dropout |
| `dropout-early.25` | 在输入层后添加Dropout (alpha=0.25) |

#### 优化器参数
| 参数 | 说明 |
|------|------|
| `faster` | 使用更快学习率 (gamma=0.1) |
| `slower` | 使用更慢学习率 (gamma=0.001) |
| `mse` | 使用MSE损失函数（而非交叉熵） |

#### 其他参数
| 参数 | 说明 |
|------|------|
| `nearest` | 使用最近舍入（round nearest） |
| `debug_ml` | ML调试模式 |
| `mux` | 使用multiplexer |

---

### mnist_full_B 专属参数

| 参数 | 说明 |
|------|------|
| `savemem` | 节省内存模式：设置 N=batch_size |

默认：N = min(N, 1000)

---

### mnist_full_C 专属参数

#### 内存参数
| 参数 | 说明 |
|------|------|
| `savemem` | 节省内存模式：设置 N=batch_size |

默认：N = min(N, max(1000, batch_size))

#### 正则化参数
| 参数 | 说明 |
|------|------|
| `batchnorm` | 在两个卷积层后添加批归一化 |
| `dropout` | 在第二个Dense层前添加Dropout(500, alpha=0.5) |
| `dropout.25` | 在第二个Dense层前添加Dropout(500, alpha=0.25) |
| `dropout.125` | 在第二个Dense层前添加Dropout(500, alpha=0.125) |
| `dropout1` | 在第一个Dense层前添加Dropout(800, alpha=0.5) |
| `dropout2` | 同时在两个Dense层前添加Dropout |

#### 其他参数
| 参数 | 说明 |
|------|------|
| `no_relu` | 移除所有ReLU激活层 |

---

### mnist_full_D 专属参数

#### 卷积层配置（必需选择）
| 参数 | 说明 |
|------|------|
| `stride1` | 使用stride=(1,1)，默认stride=(2,2) |
| `valid` | 使用VALID padding，默认SAME padding |
| `maxpool` | 在卷积层后添加MaxPool层 |

#### Dense层配置（**必需指定**）
| 参数 | 说明 |
|------|------|
| `1dense` | 使用1个Dense层：Dense(n_inner → 10) |
| `2dense` | 使用2个Dense层：Dense(n_inner → 100) + Dense(100 → 10) |

#### 正则化参数
| 参数 | 说明 |
|------|------|
| `dropout` | 在flatten后添加Dropout（1dense模式）或在两处添加（2dense模式） |
| `dropout1` | 在第一个Dense层后添加Dropout（仅2dense模式） |

**注意：** mnist_full_D 必须指定 `1dense` 或 `2dense` 参数，否则会抛出异常。

---

## 使用示例

```bash
# 网络A：训练50轮，batch_size=128，使用dropout
./compile.py mnist_full_A 50 128 -- dropout

# 网络B：调试模式，训练10轮
./compile.py mnist_full_B 10 64 -- debug

# 网络C：使用batchnorm和dropout，4线程
./compile.py mnist_full_C 100 128 4 -- batchnorm dropout

# 网络D：2个dense层，使用maxpool和dropout
./compile.py mnist_full_D 50 128 -- 2dense maxpool dropout
```

---

## 选择建议

| 场景 | 推荐网络 | 理由 |
|------|---------|------|
| 快速原型测试 | D + 1dense | 参数最少，编译最快 |
| 资源受限环境 | A 或 D | 内存占用小 |
| 性能基准测试 | B | 平衡性能与资源消耗 |
| 追求最高准确率 | C (LeNet) | 最强模型容量 |
| 消融实验 | A | 支持最丰富的配置选项 |

---

## 数据准备 ⚠️ **重要**

### ❌ 常见错误

运行时如果遇到以下错误：
```
Fatal error: not enough inputs in Player-Data/Input-P0-0
```

说明**缺少训练数据文件**。这些程序需要预处理好的 MNIST 数据。

---

### ✅ 解决方案

#### **方案 1：自动准备数据（推荐）** 🌟

使用提供的脚本自动下载并准备数据：

```bash
# 在项目根目录运行
cd /root/Covert-3PC

# 完整数据集（60000训练 + 10000测试）
python3 prepare_mnist_data.py

# 调试模式 - 根据编译参数匹配
python3 prepare_mnist_data.py 1000 1000  # debug1000
python3 prepare_mnist_data.py 100 100    # debug
python3 prepare_mnist_data.py 5000 5000  # debug5000
```

脚本会：
1. 自动下载 MNIST 数据集（通过 PyTorch）
2. 转换为 MPC 所需格式
3. 写入 `Player-Data/Input-P0-0`

**⚠️ 重要：** 数据样本数必须与编译参数匹配！
- 使用 `debug1000` 编译时，必须运行 `python3 prepare_mnist_data.py 1000 1000`
- 使用 `debug` 编译时，必须运行 `python3 prepare_mnist_data.py 100 100`
- 默认（无debug标志）时，运行 `python3 prepare_mnist_data.py`

**完成后即可运行实验**：

```bash
# 示例：debug1000 模式
python3 prepare_mnist_data.py 1000 1000
./compile.py mnist_full_A 1 128 16 debug1000
./Scripts/ring.sh mnist_full_A-1-128-16-debug1000
```

---

#### **方案 2：使用 PyTorch 集成版本**

改用自动处理数据的版本（无需手动准备）：

```bash
# 这些程序会自动下载数据
./compile.py torch_mnist_dense 1 128 16 torch
./compile.py keras_mnist_dense 1 128 torch

# 编译后运行
./Scripts/ring.sh torch_mnist_dense-1-128-16-torch
```

**优势：**
- 无需手动准备数据
- 代码更现代化
- 支持更灵活的网络定义

---

#### **方案 3：手动准备（高级用户）**

参考官方仓库：https://github.com/csiro-mlai/mnist-mpc

1. 克隆数据准备工具
2. 下载 MNIST 原始文件（ubyte 格式）
3. 运行转换脚本
4. 复制到 `Player-Data/Input-P0-0`

---

## 完整实验流程

### 第一次运行（快速验证）

```bash
# 1. 准备数据（完整数据集）
python3 prepare_mnist_data.py

# 2. 编译程序（1个epoch，16线程）
./compile.py mnist_full_A 1 128 16

# 3. 运行实验
./Scripts/ring.sh mnist_full_A-1-128-16
```

### 调试模式（推荐用于测试）

```bash
# 1. 准备调试数据（1000个样本）
python3 prepare_mnist_data.py 1000 1000

# 2. 编译程序（使用debug1000标志）
./compile.py mnist_full_C 1 128 16 debug1000 savemem trunc_pr split

# 3. 运行实验
./Scripts/ring.sh mnist_full_C-1-128-16-debug1000-savemem-trunc_pr-split
```

### 标准实验

```bash
# 数据已准备好，直接运行
./compile.py mnist_full_A 50 128 16 2dense
./Scripts/ring.sh mnist_full_A-50-128-16-2dense
```

---

**参考资料：**
- 官方数据准备工具：https://github.com/csiro-mlai/mnist-mpc
- MP-SPDZ 机器学习文档：`doc/machine-learning.rst`
