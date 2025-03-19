# 数字化模拟项目使用指南

## 一、基本用法

### 1.1 命令行选项

```bash
./bin/digitize [options]
```

参数说明:
- `-h, --help`：显示帮助信息
- `-n, --events <number>`：设置模拟事件数(默认: 100000)
- `-d, --digitizer <type>`：运行指定的数字化器(Scintillation/SiPM/ADC/Total)
- `-a, --all`：运行所有数字化器
- `-o, --output <prefix>`：设置输出文件前缀
- `-s, --seed <number>`：设置随机数种子
- `-p, --param <name> <value>`：设置参数值
- `-l, --load <file>`：从文件加载参数
- `-w, --save <file>`：保存参数到文件
- `-e, --energy-points <file>`：从文件加载能量点
- `-c, --config <file>`：加载配置文件

### 1.2 基本运行示例

```bash
# 运行所有数字化器
./bin/digitize --all

# 仅运行闪烁体数字化器
./bin/digitize --digitizer Scintillation

# 运行完整数字化链，处理50万个事件
./bin/digitize --digitizer Total --events 500000

# 设置输出文件前缀
./bin/digitize --all --output mytest
# 将生成以下文件:
# - mytest_Scintillation.root
# - mytest_SiPM.root
# - mytest_ADC.root
# - mytest_Total.root
```

### 1.3 自定义安装位置

如果在编译时指定了自定义安装目录，请相应调整命令路径：

```bash
# 假设安装到了 $HOME/.local
$HOME/.local/bin/digitize --help

# 或添加到PATH环境变量
export PATH=$PATH:$HOME/.local/bin
digitize --help
```

## 二、参数配置

### 2.1 通过命令行设置参数

```bash
# 设置单个参数
./bin/digitize --all --param EcalASICNoiseSigma 0.5

# 设置多个参数
./bin/digitize --all --param EcalSiPMPDE 0.4 --param EcalSiPMCT 0.02
```

### 2.2 使用配置文件

可以使用配置文件一次性设置多个参数：

```bash
# 从配置文件加载参数
./bin/digitize --all --load configs/custom.conf

# 或使用配置文件简写选项
./bin/digitize --all --config configs/custom.conf
```

配置文件示例(configs/custom.conf):
```
# 晶体参数
EcalMIPEnergy = 10.0      # MIP能量(MeV)
EcalCryMipLY = 120        # MIP光产额

# SiPM参数
EcalSiPMPDE = 0.4         # 光电探测效率
EcalSiPMCT = 0.02         # 串扰概率
EcalSiPMDarkRate = 1.0    # 暗计数率(MHz)

# ADC参数
EcalASICNoiseSigma = 0.5  # ASIC噪声(MeV)
```

### 2.3 保存当前参数

可以将当前使用的参数保存到文件中：

```bash
# 保存默认参数
./bin/digitize --save params/default.conf

# 修改参数后保存
./bin/digitize --param EcalSiPMPDE 0.38 --save params/modified.conf
```

## 三、结果分析与可视化

### 3.1 使用ROOT脚本绘制图形

数字化运行后生成ROOT文件，可以使用ROOT脚本生成图形：

```bash
# 绘制单个数字化器的结果
root -l 'scripts/plot_results.C("digi_out_Total.root", "plots/total")'

# 自定义输出格式
root -l 'scripts/plot_results.C("digi_out_SiPM.root", "plots/sipm")'

# 比较不同数字化器的分辨率
root -l 'scripts/compare_digitizers.C("digi_out_Scintillation.root", "digi_out_SiPM.root", "digi_out_ADC.root", "digi_out_Total.root", "comparison")'
```

ROOT脚本生成的图形包括：
- 能量分辨率随能量变化曲线
- 各能量点的响应直方图
- 能量线性度曲线

### 3.2 参数扫描结果绘图

```bash
# 绘制参数扫描结果
root -l 'scripts/plot_scan_results.C("param_scan/scan_EcalASICNoiseSigma.root", "param_scan")'
```

## 四、高级功能

### 4.1 参数扫描

可以对特定参数进行扫描，分析参数变化对探测器性能的影响：

```bash
# 使用脚本进行参数扫描
./scripts/run_all.sh

# 或手动执行扫描
./bin/digitize --scan EcalASICNoiseSigma 0.1 0.2 0.3 0.4 0.5 --output scan_noise
```

### 4.2 自定义能量点

默认情况下，数字化模拟使用预设的能量点。您可以自定义能量点：

```bash
# 从文件加载能量点
./bin/digitize --all --energy-points configs/energy_points.txt

# 能量点文件格式示例(configs/energy_points.txt):
# 10
# 20
# 50
# 100
# 200
# 500
# 1000
```

### 4.3 开发自定义数字化器

您可以通过继承`DigitizationBase`类来实现自定义的数字化器：

```cpp
class MyDigitizer : public DigitizationBase {
public:
    MyDigitizer() : DigitizationBase("MyDigitizer") {}
    
    // 实现自定义数字化方法
    double digitize(double energy) override {
        // 自定义数字化逻辑
        return energy * someFunction();
    }

protected:
    // 定义Tree分支
    void initializeTree() override {
        dataTree = std::make_unique<TTree>("myEvents", "My Digitizer Events");
        // 添加分支...
    }
};
```

## 五、故障排除

### 5.1 常见问题

1. **ROOT库问题**：确保ROOT环境变量正确设置
   ```bash
   source /path/to/root/bin/thisroot.sh
   ```

2. **编译错误**：如果遇到标准不匹配警告，尝试指定C++标准
   ```bash
   cmake .. -DCMAKE_CXX_STANDARD=17
   ```

3. **安装权限问题**：使用自定义安装路径
   ```bash
   cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/.local
   ```

### 5.2 运行测试

提供了测试脚本验证安装和功能：

```bash
# 运行所有测试
./scripts/run_tests.sh

# 运行特定测试
./build/bin/test_parameters
```

---

如需更多帮助，请参考项目README文件中的联系方式。 