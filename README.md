# 数字化模拟项目

## 项目概述

这个项目提供了一个模块化的粒子探测器数字化模拟框架，用于模拟粒子能量沉积从物理信号到数字信号的转换过程。系统将整个数字化过程分为三个主要阶段：

1. **闪烁体光子产生**：模拟入射粒子在闪烁晶体中产生的光子
2. **SiPM光电转换**：模拟SiPM（硅光电倍增器）对光子的探测和转换
3. **ADC模数转换**：模拟前端电子学和ADC对电信号的处理和数字化

系统既可以评估完整的数字化流程，也可以单独评估每个数字化步骤，方便分析各个环节对最终能量分辨率的影响。

## 项目特点

- **模块化设计**：各数字化阶段独立实现，便于单独测试和评估
- **参数化配置**：通过配置文件或命令行可灵活调整探测器参数
- **快速模拟**：高效实现，可处理大量能量点和事件数
- **详细分析**：提供分辨率、线性度等关键性能指标分析
- **ROOT集成**：使用ROOT框架进行数据存储和分析
- **C++/ROOT绘图**：使用C++/ROOT脚本生成高质量图表

## 安装指南

### 依赖项

- C++17兼容的编译器（GCC 8+或Clang 10+）
- CMake 3.10+
- ROOT 6.20+（带有Core、RIO、Hist、Tree组件）

### 编译步骤

```bash
# 1. 克隆仓库
git clone https://github.com/yourusername/digitization.git
cd digitization/tools/digitization

# 2. 创建并进入构建目录
mkdir build && cd build

# 3. 配置项目（可选：指定安装目录）
cmake ..                               # 默认安装到/usr/local/
# 或者指定自定义安装路径
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/.local  # 安装到用户目录

# 4. 编译
make

# 5. 安装（可选）
make install
```

### 快速设置（推荐）

为方便使用，项目提供了一键环境设置脚本：

```bash
# 在项目根目录下运行
cd tools/digitization
source ./setup.sh

# 设置脚本会：
# - 检查并设置ROOT环境
# - 将编译目录或安装目录添加到PATH
# - 创建必要的输出目录
# - 设置环境变量
```

### 可执行文件位置

根据您的编译和安装方式，可执行文件位于不同位置：

1. **编译后不安装**：可执行文件位于 `build/bin/digitize`
2. **安装后**：可执行文件位于您指定的安装目录，例如 `$HOME/.local/bin/digitize`

### 测试安装

```bash
# 使用setup.sh后可直接运行
digitize --help  # 显示帮助信息

# 或使用编译目录的可执行文件
./build/bin/digitize --help

# 或使用安装后的可执行文件
$HOME/.local/bin/digitize --help  # 使用自定义安装路径
```

## 基本用法

数字化模拟程序提供多种运行模式和配置选项：

```bash
# 运行所有数字化器，使用默认参数
digitize --all

# 仅运行ADC数字化器，使用100万个事件
digitize --digitizer ADC --events 1000000

# 设置随机数种子并指定输出文件前缀
digitize --all --seed 12345 --output results/mytest

# 从配置文件加载参数
digitize --all --load configs/custom.conf

# 设置单个参数
digitize --all --param EcalSiPMNoise 3.2
```

### 输入和输出文件

- **配置文件**：通常位于 `configs/` 目录
- **输出文件**：默认保存在当前工作目录，建议使用 `--output` 指定输出路径
- **创建目录**：如需保存到子目录，请确保先创建该目录：`mkdir -p results`

### 绘制结果

使用ROOT脚本绘制结果：

```bash
# 绘制数字化结果
root -l 'scripts/plot_results.C("results/mytest_Total.root", "results/plots/total")'

# 比较不同数字化器的分辨率
root -l 'scripts/compare_digitizers.C("results/mytest_Scintillation.root", "results/mytest_SiPM.root", "results/mytest_ADC.root", "results/mytest_Total.root", "results/comparison")'

# 绘制参数扫描结果
root -l 'scripts/plot_scan_results.C("results/scan_EcalASICNoiseSigma.root", "results/scan_plots")'
```

### 推荐的工作流程

```bash
# 1. 设置环境
source ./setup.sh

# 2. 创建输出目录
mkdir -p results/plots

# 3. 运行模拟
digitize --all --output results/run1

# 4. 分析结果
root -l 'scripts/plot_results.C("results/run1_Total.root", "results/plots/run1")'
```

详细用法请参考[USAGE.md](docs/USAGE.md)文档。

## 许可证

本项目采用[MIT许可证](LICENSE)。

## 联系方式

如有问题或建议，请通过以下方式联系：

- 项目维护者：您的姓名
- 电子邮箱：您的邮箱
- 项目Issues：https://github.com/yourusername/digitization/issues

## 致谢

- 感谢ROOT团队提供的强大数据分析和可视化框架
- 感谢所有为该项目提供反馈和建议的用户

---

© 2023 数字化模拟项目团队 