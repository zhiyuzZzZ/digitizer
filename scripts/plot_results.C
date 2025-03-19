/*
 * 数字化结果绘图脚本
 * 
 * 使用方法:
 * root -l 'scripts/plot_results.C("results/mytest_Total.root", "results/plots/total")'
 */

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TStyle.h"
#include "TLegend.h"
#include "TPaveText.h"
#include "TGraphErrors.h"
#include "TMath.h"
#include "TString.h"
#include <iostream>

void plot_results(const char* filename, const char* outputDir = "plots") {
  // 打开输入文件
  TFile *file = new TFile(filename, "READ");
  
  if (!file || file->IsZombie()) {
    std::cerr << "无法打开文件: " << filename << std::endl;
    return;
  }
  
  // 获取树
  TTree *tree = (TTree*)file->Get("events");
  
  if (!tree) {
    std::cerr << "无法找到events树" << std::endl;
    file->Close();
    return;
  }
  
  // 创建输出目录
  gSystem->mkdir(outputDir, kTRUE);
  
  // 设置绘图样式
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(1);
  
  // 创建画布
  TCanvas *canvas = new TCanvas("canvas", "数字化结果", 800, 600);
  
  // 创建能量分辨率图
  TGraphErrors *resGraph = new TGraphErrors();
  resGraph->SetTitle("能量分辨率;能量 (MeV);分辨率 (%)");
  resGraph->SetMarkerStyle(20);
  resGraph->SetMarkerColor(kBlue);
  resGraph->SetLineColor(kBlue);
  
  // 获取能量点列表
  float energy, amplitude;
  tree->SetBranchAddress("trueE", &energy);
  tree->SetBranchAddress("amplitude", &amplitude);
  
  // 获取能量点列表
  std::vector<float> energyPoints;
  for (int i = 0; i < tree->GetEntries(); i++) {
    tree->GetEntry(i);
    if (std::find(energyPoints.begin(), energyPoints.end(), energy) == energyPoints.end()) {
      energyPoints.push_back(energy);
    }
  }
  
  // 对每个能量点创建直方图并拟合
  int pointIndex = 0;
  for (float e : energyPoints) {
    // 创建直方图
    TString histName = TString::Format("h_%.1f", e);
    TH1F *hist = new TH1F(histName, TString::Format("E = %.1f MeV", e), 100, 0, 0);
    
    // 填充直方图
    TString cut = TString::Format("trueE == %.6f", e);
    tree->Draw(TString::Format("amplitude>>%s", histName.Data()), cut);
    
    // 自动调整范围
    if (hist->GetEntries() > 0) {
      double mean = hist->GetMean();
      double rms = hist->GetRMS();
      hist->SetAxisRange(mean - 5*rms, mean + 5*rms, "X");
    }
    
    // 拟合高斯
    hist->Fit("gaus", "Q");
    TF1 *fit = hist->GetFunction("gaus");
    
    if (fit) {
      double mean = fit->GetParameter(1);
      double sigma = fit->GetParameter(2);
      double resolution = 100.0 * sigma / mean;
      
      // 添加到分辨率图
      resGraph->SetPoint(pointIndex, e, resolution);
      resGraph->SetPointError(pointIndex, 0, resolution/sqrt(hist->GetEntries()));
      
      // 在直方图上添加拟合信息
      TPaveText *pt = new TPaveText(0.65, 0.65, 0.89, 0.89, "NDC");
      pt->AddText(TString::Format("Mean = %.2f", mean));
      pt->AddText(TString::Format("Sigma = %.2f", sigma));
      pt->AddText(TString::Format("Res = %.2f%%", resolution));
      pt->SetFillColor(0);
      pt->SetBorderSize(0);
      pt->Draw();
      
      // 保存直方图
      canvas->SaveAs(TString::Format("%s/hist_%.1fMeV.png", outputDir, e));
      canvas->SaveAs(TString::Format("%s/hist_%.1fMeV.pdf", outputDir, e));
      
      pointIndex++;
    }
    
    delete hist;
  }
  
  // 绘制分辨率图
  canvas->Clear();
  resGraph->Draw("AP");
  
  // 拟合分辨率曲线 R = a/sqrt(E) + b
  TF1 *resFit = new TF1("resFit", "TMath::Sqrt([0]*[0]/(x) + [1]*[1])", energyPoints.front(), energyPoints.back());
  resFit->SetParameters(1.0, 0.5);
  resGraph->Fit(resFit, "R");
  
  // 添加拟合信息
  TPaveText *ptRes = new TPaveText(0.65, 0.75, 0.89, 0.89, "NDC");
  ptRes->AddText(TString::Format("a = %.2f%%", resFit->GetParameter(0)));
  ptRes->AddText(TString::Format("b = %.2f%%", resFit->GetParameter(1)));
  ptRes->AddText(TString::Format("R = #frac{%.2f}{#sqrt{E}} #oplus %.2f%%", 
                                resFit->GetParameter(0), resFit->GetParameter(1)));
  ptRes->SetFillColor(0);
  ptRes->SetBorderSize(0);
  ptRes->Draw();
  
  // 保存分辨率图
  canvas->SaveAs(TString::Format("%s/resolution.png", outputDir));
  canvas->SaveAs(TString::Format("%s/resolution.pdf", outputDir));
  
  // 关闭文件
  file->Close();
  
  std::cout << "绘图完成，结果保存在: " << outputDir << std::endl;
} 