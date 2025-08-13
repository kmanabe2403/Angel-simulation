#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TF1.h"
#include "TMath.h"

{
    // CSVファイル名
    std::string filename = "fit_data.csv";
    std::string picnum = "_ver1_1_20250616";

    // データ格納用ベクター
    std::vector<double> x_vals, y_vals;
    std::vector<double> ex_low, ex_high;
    std::vector<double> ey_low, ey_high;

    // ファイル読み込み用
    std::ifstream file(filename);
    // if (!file.is_open()) {
    //     std::cerr << "Error: failed to open " << filename << std::endl;
    //     return 1;
    // }

    std::string line;
    // CSVは行数 = num、1行に5点分のデータ（各点で8項目ずつ）あると仮定
    // 1列目：num（index）、2~6列目：tate (5点)
    // 7~11：yoko、12~16：sim_tate、17~21：tate_plus_error、22~26：tate_minus_error
    // 27~31：yoko_plus_error、32~36：yoko_minus_error
    // → 1行に 37列ある想定

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        std::vector<double> row_vals;

        while (std::getline(ss, cell, ',')) {
            try {
                row_vals.push_back(std::stod(cell));
            } catch (...) {
                // 空セルや変換失敗は無視
                row_vals.push_back(0);
            }
        }

        if (row_vals.size() < 36) {
            std::cerr << "Warning: unexpected columns (" << row_vals.size() << ")" << std::endl;
            continue;
        }

        // 1行で5点分のデータを展開
        for (int j = 0; j < 5; j++) {
            double y = row_vals[1 + j];         // tate[i][j]
            double x = row_vals[6 + j];         // yoko[i][j]
            double ey_p = row_vals[16 + j];     // tate_plus_error[i][j]
            double ey_m = row_vals[21 + j];     // tate_minus_error[i][j]
            double ex_p = row_vals[26 + j];     // yoko_plus_error[i][j]
            double ex_m = row_vals[31 + j];     // yoko_minus_error[i][j]

            x_vals.push_back(x);
            y_vals.push_back(y);
            ex_low.push_back(ex_m);
            ex_high.push_back(ex_p);
            ey_low.push_back(ey_m);
            ey_high.push_back(ey_p);
        }
    }
    file.close();

    int npoints = x_vals.size();

    // TGraphAsymmErrorsの作成
    TGraphAsymmErrors *graph = new TGraphAsymmErrors(npoints);

    for (int i = 0; i < npoints; i++) {
        if (y_vals[i] > 0){
            graph->SetPoint(i, x_vals[i], y_vals[i]);
            // std::cout << "i" << i << ", x" << x_vals[i] << ", y" << y_vals[i] <<std::endl;
            graph->SetPointError(i, ex_low[i], ex_high[i], ey_low[i], ey_high[i]);
        }

    }

    // フィット関数定義: a + b * cos^n(x*pi/180)
    TF1 *fit_func = new TF1("fit_func", "[0] + [1]*TMath::Power(TMath::Cos(x*TMath::Pi()/180), [2])", 5, 90);
    fit_func->SetParameters(50, 100, 2);  // 初期値

    // フィット実行
    gStyle->SetOptFit(1111);  // ←★これを追加！
    graph->Fit(fit_func, "R");

    // キャンバスに描画
    TCanvas *c1 = new TCanvas("c1", "Fit example", 800, 600);
    graph->GetXaxis()->SetLimits(0, 90);
    graph->SetMinimum(-25);
	graph->SetMaximum(150);

    graph->SetMarkerStyle(20);     // ●マーカー
    graph->SetMarkerColor(kBlue);  // 青色に
    graph->SetLineColor(kBlue);    // エラーバーも青色に
  
    graph->SetTitle("ver1");
    graph->Draw("AP");
    fit_func->Draw("SAME");
    c1->Update();

    // TPaveStats* stats = (TPaveStats*)graph->GetListOfFunctions()->FindObject("stats");
    // if (stats) {
    //     stats->SetX1NDC(0.65); // 左端
    //     stats->SetX2NDC(0.88); // 右端
    //     stats->SetY1NDC(0.65); // 下端
    //     stats->SetY2NDC(0.85); // 上端
    //     stats->SetTextSize(0.03);
    //     stats->Draw(); // 明示的に再描画
    // }


    std::string picname = std::string("./pic/fit/fit") + picnum + ".png";
    c1->SaveAs(picname.c_str());

    std::cout << "Fit parameters:" << std::endl;
    std::cout << "a = " << fit_func->GetParameter(0) << " ± " << fit_func->GetParError(0) << std::endl;
    std::cout << "b = " << fit_func->GetParameter(1) << " ± " << fit_func->GetParError(1) << std::endl;
    std::cout << "n = " << fit_func->GetParameter(2) << " ± " << fit_func->GetParError(2) << std::endl;

    return 0;
}
