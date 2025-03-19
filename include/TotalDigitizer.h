#ifndef TOTAL_DIGITIZER_H
#define TOTAL_DIGITIZER_H

#include "DigitizationBase.h"
#include <TH1D.h>
#include <memory>

class TotalDigitizer : public DigitizationBase {
public:
    TotalDigitizer();
    virtual ~TotalDigitizer() = default;
    
    // 数字化方法
    double digitize(double energy) override;
    
    // 重载运行方法，添加等效噪声能量(ENE)计算
    virtual void run(int nEvents = 100000) override;
    
    // 重载保存方法，添加ENE直方图保存
    virtual void saveResults(const std::string& outputFile) override;
    
    // 删除或修改plotResults方法 - 它不是基类的方法，不应标记为override
    void plotResults(const std::string& outputPrefix);
    
protected:
    // 覆盖树初始化方法
    void initializeTree() override;
    
    // 添加：覆盖均匀抽样树初始化方法
    void initializeSamplingTree() override;
    
private:
    // 用于存储Tree数据的变量
    double inputEnergy;
    double phScin;
    double phScinAtt;
    double phScinAttLYRand;
    double peSiPM;
    double peSiPMSat;
    double dc;
    double dcCT;
    double peSiPMSatDark;
    double peSiPMSatDarkGainFlu;
    double peSiPMSatDarkGainFluPedSub;
    double peSiPMSatDarkGainFluPedSubCut;
    double adcInitial;
    double adcGainCorr;
    double gainMode;
    double gain;
    double noiseFEE;
    double noiseASIC;
    double pedMean;
    double outputEnergy;
    
    // 等效噪声能量直方图
    std::unique_ptr<TH1D> h_ENE;
    
    // 计算等效噪声能量
    void calculateENE();
};

#endif // TOTAL_DIGITIZER_H 