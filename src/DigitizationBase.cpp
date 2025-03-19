#include "DigitizationBase.h"
#include <iostream>
#include <cmath>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TFitResult.h>

DigitizationBase::DigitizationBase(const std::string& name) 
    : moduleName(name), params(DetectorParameters::getInstance()), rand(0) {
    
    // 初始化能量点
    energies = params.getEnergyPoints();
    if (energies.empty()) {
        // 默认能量点
        energies = {0.5*0.89, 0.89, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 30000};
    }
    
    // 初始化直方图和函数
    initializeHistograms();
    initializeFunctions();
    
    // 初始化数据树
    initializeDataTrees();
}

DigitizationBase::~DigitizationBase() {
    // 智能指针会自动清理资源
}

void DigitizationBase::initializeHistograms() {
    // 释放旧直方图防止内存泄漏
    h_Energies.clear();
    
    // 使用模块名称前缀确保直方图名称唯一
    std::string prefix = moduleName + "_";
    
    // 二维直方图用于观察能量响应
    h2_dynamic = std::make_unique<TH2D>((prefix + "h2_dynamic").c_str(), 
                                       "Energy Response;Input Energy [MeV];Digitized Energy [MeV]", 
                                       200, 0, energies.empty() ? 1000 : energies.back() * 1.2, 
                                       200, 0, energies.empty() ? 1000 : energies.back() * 1.5);
    
    // 为每个能量点创建直方图
    for (size_t i = 0; i < energies.size(); ++i) {
        double energy = energies[i];
        double histMin = energy * 0.7;
        double histMax = energy * 1.3;
        int nBins = 100;
        
        // 调整低能量点的直方图范围
        if (energy < 20) {
            histMin = energy * 0.5;
            histMax = energy * 1.5;
            nBins = 80;
        }
        
        // 添加模块名称前缀确保直方图名称唯一
        std::string histName = prefix + "h_" + std::to_string(energy) + "_MeV";
        std::string histTitle = std::to_string(energy) + " MeV Energy Distribution";
        
        // 创建直方图并添加到容器
        h_Energies.push_back(std::make_unique<TH1D>(
            histName.c_str(), histTitle.c_str(), 
            nBins, histMin, histMax
        ));
    }
    
    std::cout << "已初始化 " << h_Energies.size() << " 个能量直方图" << std::endl;
    
    // 初始化数据Tree
    initializeTree();
}

void DigitizationBase::initializeFunctions() {
    // SiPM响应函数
    f_SiPMResponse = std::make_unique<TF1>(
        "f_SiPMResponse", 
        "((1-[1])*[0]*(1-exp(-x/[0]))+[1]*x)*([2]+1)/([2]+x/([0]*(1-exp(-x/[0]))))*(1+[3]*exp(-x/[0]))", 
        0, 1e+9
    );
    f_SiPMResponse->SetParameters(
        6.19783e+06, 
        5.08847e-01, 
        1.27705e+01, 
        params.getParameter("EcalSiPMCT")
    );
    
    // SiPM sigma探测函数
    f_SiPMSigmaDet = std::make_unique<TF1>(
        "f_SiPMSigmaDet", 
        "8.336e-01*sqrt(x+0.2379)", 
        0, 1e+9
    );
    
    // SiPM sigma响应函数
    f_SiPMSigmaRecp = std::make_unique<TF1>(
        "f_SiPMSigmaRecp", 
        "f_SiPMResponse+f_SiPMSigmaDet", 
        0, 1e+9
    );
    
    f_SiPMSigmaRecm = std::make_unique<TF1>(
        "f_SiPMSigmaRecm", 
        "f_SiPMResponse-f_SiPMSigmaDet", 
        0, 1e+9
    );
    
    // 非对称高斯函数
    f_AsymGauss = std::make_unique<TF1>(
        "AsymGauss", 
        [](double *x, double *par) -> double {
            double val = x[0];
            double mean = par[0];
            double sigma_left = par[1];
            double sigma_right = par[2];

            if (val < mean) {
                return exp(-0.5 * pow((val - mean) / sigma_left, 2));
            } else {
                return exp(-0.5 * pow((val - mean) / sigma_right, 2));
            }
        }, 
        -10, 10, 3
    );
    
    // 暗噪声函数
    f_DarkNoise = std::make_unique<TF1>(
        "f_DarkNoise", 
        "pow([0]*x, x-1) * exp(-[0]*x) / TMath::Factorial(x)"
    );
    f_DarkNoise->SetParameter(0, params.getParameter("EcalSiPMCT"));
}

void DigitizationBase::initializeTree() {
    // 创建一个通用的事件树
    dataTree = std::make_unique<TTree>("events", "Digitization Events");
    
    // 如果启用了均匀抽样，也初始化抽样树
    if (uniformSampling) {
        initializeSamplingTree();
    }
}

void DigitizationBase::initializeDataTrees() {
    // 清空数据结构
    resolutionData.energy.clear();
    resolutionData.resolution.clear();
    resolutionData.resError.clear();
    
    linearityData.inputEnergy.clear();
    linearityData.responseDiff.clear();
    
    // 初始化事件树指针
    eventTreePtr = nullptr;
}

double DigitizationBase::calculateMeanCT() {
    double mean_CT = 0;
    for (int i = 1; i < 10; i++) {
        mean_CT += (i-1) * f_DarkNoise->Eval(i);
    }
    return mean_CT;
}

void DigitizationBase::run(int nEvents) {
    // 确保直方图已初始化
    initializeHistograms();
    initializeFunctions();
    initializeTree();
    
    std::cout << "运行 " << moduleName << " 数字化 (" << nEvents << " 事件)..." << std::endl;
    
    // 处理固定能量点
    for (size_t i = 0; i < energies.size(); ++i) {
        double energy = energies[i];
        std::cout << "处理能量点: " << energy << " MeV" << std::endl;
        
        // 安全检查：确保直方图存在
        if (i >= h_Energies.size() || !h_Energies[i]) {
            std::cerr << "错误：能量点 " << energy << " MeV 的直方图未初始化" << std::endl;
            continue;  // 跳过这个能量点
        }
        
        // 获取对应的直方图
        TH1D* hist = h_Energies[i].get();
        
        // 模拟nEvents个事件
        for (int j = 0; j < nEvents; ++j) {
            // 设置当前处理的能量点
            inputEnergy = energy;
            
            // 数字化
            double outputEnergy = digitize(energy);
            
            // 填充直方图
            hist->Fill(outputEnergy);
            
            // 安全检查：确保2D直方图存在
            if (h2_dynamic) {
                h2_dynamic->Fill(energy, outputEnergy);
            }
            
            // 每处理10000个事件打印一次进度
            if ((j+1) % 10000 == 0 || j == nEvents - 1) {
                std::cout << "已处理 " << j+1 << "/" << nEvents << " 事件" << std::endl;
            }
        }
    }
    
    // 计算能量分辨率
    calculateResolution();
    
    // 如果启用了均匀抽样，单独处理
    if (uniformSampling) {
        // 确保抽样范围有效
        if (samplingMinEnergy <= 0 || samplingMaxEnergy <= 0 || samplingMinEnergy >= samplingMaxEnergy) {
            std::cerr << "错误：无效的抽样范围 [" << samplingMinEnergy << ", " << samplingMaxEnergy << "]" << std::endl;
            return;
        }
        
        runUniformSampling(nEvents);
    }
}

void DigitizationBase::calculateResolution() {
    // 我们不再计算分辨率，只记录能量点信息
    // 分辨率计算会在绘图脚本中进行
    resolutionData.energy.clear();
    resolutionData.resolution.clear();
    resolutionData.resError.clear();
    
    // 仅收集直方图基本统计信息用于日志
    for (size_t i = 0; i < energies.size(); ++i) {
        // 检查直方图是否有数据
        if (h_Energies[i]->GetEntries() < 1) {
            std::cout << "警告：能量 " << energies[i] 
                      << " MeV 的直方图没有数据" << std::endl;
            continue;
        }
        
        // 只记录能量点
        resolutionData.energy.push_back(energies[i]);
        
        // 输出基本统计信息
        double mean = h_Energies[i]->GetMean();
        double rms = h_Energies[i]->GetRMS();
        std::cout << "Energy: " << energies[i] 
                  << " MeV, Mean: " << mean 
                  << ", RMS: " << rms << std::endl;
    }
    
    std::cout << "注意：分辨率计算将在绘图脚本中进行" << std::endl;
}

void DigitizationBase::saveResults(const std::string& outputFile) {
    // 创建输出文件
    TFile* file = nullptr;
    try {
        file = TFile::Open(outputFile.c_str(), "RECREATE");
        if (!file || file->IsZombie()) {
            std::cerr << "无法创建输出文件: " << outputFile << std::endl;
            return;
        }
        
        // 保存能量直方图
        for (const auto& hist : h_Energies) {
            if (hist) hist->Write();
        }
        
        // 保存事件树
        if (dataTree) dataTree->Write();
        
        // 保存均匀抽样树
        if (samplingTree) samplingTree->Write();
        
        // 添加参数保存
        try {
            // 创建参数目录
            TDirectory* paramDir = file->mkdir("Parameters");
            if (paramDir) {
                paramDir->cd();
                
                // 保存数字化器类型
                TNamed* digitizerType = new TNamed("digitizerType", moduleName.c_str());
                digitizerType->Write();
                
                // 保存均匀抽样信息
                TNamed* samplingEnabled = new TNamed("uniformSamplingEnabled", 
                                                   uniformSampling ? "true" : "false");
                samplingEnabled->Write();
                
                if (uniformSampling) {
                    char minEnergyStr[50], maxEnergyStr[50];
                    snprintf(minEnergyStr, 50, "%.6f", samplingMinEnergy);
                    snprintf(maxEnergyStr, 50, "%.6f", samplingMaxEnergy);
                    
                    TNamed* minEnergy = new TNamed("samplingMinEnergy", minEnergyStr);
                    TNamed* maxEnergy = new TNamed("samplingMaxEnergy", maxEnergyStr);
                    minEnergy->Write();
                    maxEnergy->Write();
                }
                
                // 保存运行时间
                time_t now = time(nullptr);
                char timeStr[100];
                strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&now));
                TNamed* runTime = new TNamed("runTime", timeStr);
                runTime->Write();
                
                // 创建参数树
                TTree* paramTree = new TTree("parameters", "Digitization Parameters");
                
                // 定义参数变量
                char name[100];
                double value;
                
                // 创建分支
                paramTree->Branch("name", name, "name[100]/C");
                paramTree->Branch("value", &value, "value/D");
                
                // 获取参数实例
                auto& params = DetectorParameters::getInstance();
                
                // 添加常用参数
                const char* commonParams[] = {
                    "EcalMIPEnergy", "EcalCryMipLY", "EcalCryEffLY", "EcalCryIntLY", 
                    "EcalCryIntLYFlu", "EcalCryAtt", "EcalSiPMPDE", "EcalSiPMDCR", 
                    "EcalSiPMCT", "EcalSiPMGainMean", "EcalSiPMGainSigma",
                    "EcalFEENoiseSigma", "EcalASICNoiseSigma", "ADCbit", "Pedestal", "TotalGain"
                };
                
                // 填充参数树
                for (const auto& param : commonParams) {
                    try {
                        strncpy(name, param, 99);
                        name[99] = '\0';
                        value = params.getParameter(param);
                        paramTree->Fill();
                    } catch (...) {
                        // 忽略不存在的参数
                    }
                }
                
                // 保存能量点树
                TTree* energyTree = new TTree("energyPoints", "Energy Points");
                double energy;
                energyTree->Branch("energy", &energy, "energy/D");
                
                for (double e : energies) {
                    energy = e;
                    energyTree->Fill();
                }
                
                // 写入树
                paramTree->Write();
                energyTree->Write();
                
                // 返回到主目录
                file->cd();
            }
        } catch (...) {
            std::cerr << "保存参数时发生异常，但将继续保存其他数据" << std::endl;
        }
        
        // 注释掉分辨率图的创建
        // try {
        //     calculateResolution();
        //     
        //     if (!resolutionData.energy.empty()) {
        //         int nPoints = resolutionData.energy.size();
        //         double* x = new double[nPoints];
        //         double* y = new double[nPoints];
        //         double* ex = new double[nPoints]; // x误差
        //         double* ey = new double[nPoints]; // y误差
        //         
        //         for (int i = 0; i < nPoints; i++) {
        //             x[i] = resolutionData.energy[i];
        //             y[i] = resolutionData.resolution[i];
        //             ex[i] = 0.0; // 无x误差
        //             ey[i] = resolutionData.resError[i];
        //         }
        //         
        //         // 创建图形
        //         TGraphErrors* g_Resolution = new TGraphErrors(nPoints, x, y, ex, ey);
        //         g_Resolution->SetName("g_Resolution");
        //         g_Resolution->SetTitle("Energy Resolution;Energy [MeV];#sigma/<E_{rec}> [%]");
        //         
        //         // 写入图形
        //         g_Resolution->Write();
        //         
        //         // 清理内存
        //         delete[] x;
        //         delete[] y;
        //         delete[] ex;
        //         delete[] ey;
        //     }
        // } catch (...) {
        //     std::cerr << "保存分辨率图时发生异常，但将继续保存其他数据" << std::endl;
        // }
        
        // 关闭文件
        file->Close();
        delete file;
        
        std::cout << "结果已保存到: " << outputFile << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "保存结果时发生异常: " << e.what() << std::endl;
        if (file) {
            file->Close();
            delete file;
        }
    }
    catch (...) {
        std::cerr << "保存结果时发生未知异常" << std::endl;
        if (file) {
            file->Close();
            delete file;
        }
    }
}

void DigitizationBase::saveParametersToFile(TFile* file) {
    // 这个方法暂时不会被调用
    if (!file || file->IsZombie()) return;
    
    try {
        // 只保存最基本的信息
        TNamed* digitizerType = new TNamed("digitizerType", moduleName.c_str());
        digitizerType->Write();
    }
    catch (...) {
        // 忽略所有异常
    }
}

std::vector<TH1D*> DigitizationBase::getEnergyHistograms() const {
    std::vector<TH1D*> result;
    for (const auto& hist : h_Energies) {
        result.push_back(hist.get());
    }
    return result;
}

TH1D* DigitizationBase::getResponseHistogram(double energy) const {
    // 查找最接近请求能量的直方图
    for (size_t i = 0; i < energies.size(); i++) {
        if (std::abs(energies[i] - energy) < 0.01) {
            return h_Energies[i].get();
        }
    }
    return nullptr;
}

TGraphErrors* DigitizationBase::getResolutionGraph() const {
    std::cout << "警告: getResolutionGraph() 方法已弃用，请使用绘图脚本进行分辨率计算" << std::endl;
    return nullptr;
}

// 添加默认的initializeSamplingTree实现
void DigitizationBase::initializeSamplingTree() {
    // 创建一个通用的抽样树
    samplingTree = std::make_unique<TTree>("sampling", "Uniform Sampling Events");
}

// 执行均匀能量抽样
void DigitizationBase::runUniformSampling(int nEvents) {
    std::cout << "执行均匀能量抽样 (" << nEvents << " 事件)..." << std::endl;
    std::cout << "能量范围: [" << samplingMinEnergy << ", " << samplingMaxEnergy << "] MeV" << std::endl;
    
    // 初始化均匀抽样树
    initializeSamplingTree();
    
    // 创建2D直方图记录输入和输出能量
    TH2D* h2_sampling = new TH2D("h2_sampling", "Uniform Sampling;Input Energy [MeV];Output Energy [MeV]",
                                100, samplingMinEnergy, samplingMaxEnergy, 
                                100, 0, samplingMaxEnergy * 1.5);
    
    // 均匀抽样能量点并进行数字化
    for (int i = 0; i < nEvents; i++) {
        try {
            // 均匀抽样输入能量
            double samplingInputEnergy = rand.Uniform(samplingMinEnergy, samplingMaxEnergy);
            
            // 保存原始输入能量
            double originalInputEnergy = inputEnergy;
            
            // 设置新的输入能量
            inputEnergy = samplingInputEnergy;
            
            // 数字化
            double samplingOutputEnergy = digitize(samplingInputEnergy);
            
            // 恢复原始输入能量
            inputEnergy = originalInputEnergy;
            
            // 填充2D直方图
            if (h2_sampling) {
                h2_sampling->Fill(samplingInputEnergy, samplingOutputEnergy);
            }
            
            // 填充均匀抽样树
            if (samplingTree) {
                samplingTree->Fill();
            }
            
            // 每处理10000个事件打印一次进度
            if ((i+1) % 10000 == 0 || i == nEvents - 1) {
                std::cout << "已处理 " << i+1 << "/" << nEvents << " 事件" << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "处理均匀抽样时发生异常: " << e.what() << std::endl;
            continue;  // 跳过这个事件
        } catch (...) {
            std::cerr << "处理均匀抽样时发生未知异常" << std::endl;
            continue;  // 跳过这个事件
        }
    }
    
    // 保存2D直方图
    if (h2_sampling) {
        h2_sampling->SetDirectory(nullptr); // 从当前目录分离
        histograms2D.push_back(h2_sampling);
    }
    
    std::cout << "均匀能量抽样完成" << std::endl;
} 