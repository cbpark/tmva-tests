#include <iostream>
#include <string>
#include "TFile.h"
#include "TMVA/CrossValidation.h"
#include "TMVA/DataLoader.h"
#include "TMVA/TMVAGui.h"
#include "TTree.h"

const std::string APPNAME = "ttbarCrossValidation";

void ttbarCrossValidation() {
    std::cout << "==> Start " << APPNAME << '\n';

    TFile input{"ttbar-analysis/ttbar_analysis.root"};
    std::cout << "--- " << APPNAME
              << "  : Using input file: " << input.GetName() << '\n';

    auto dataloader = new TMVA::DataLoader("dataset");
    dataloader->AddVariable("mt2", "M_{T2}", "", 'F');
    dataloader->AddVariable("mbl", "m_{bl}", "", 'F');
    dataloader->AddVariable("mbbll", "m_{bbll}", "", 'F');

    auto sig_tree = dynamic_cast<TTree *>(input.Get("TreeS"));
    dataloader->AddSignalTree(sig_tree, 1.0);
    auto bkg_tree = dynamic_cast<TTree *>(input.Get("TreeB"));
    dataloader->AddBackgroundTree(bkg_tree, 1.0);

    dataloader->PrepareTrainingAndTestTree("", "SplitMode=random:!V");

    auto outfile = TFile::Open("ttbar_tmva_cv.root", "recreate");
    TMVA::CrossValidation cv{
        APPNAME, dataloader, outfile,
        "!V:!Silent:!Correlations:AnalysisType=Classification:NumFolds=2"};

    cv.BookMethod(
        TMVA::Types::kBDT, "BDT",
        "!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:"
        "AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:"
        "SeparationType=GiniIndex:nCuts=20");

    cv.BookMethod(TMVA::Types::kMLP, "NN",
                  "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:"
                  "HiddenLayers=N+1,N:TestRate=5:!UseRegulator");

    cv.Evaluate();

    std::cout << "==> Wrote root file: " << outfile->GetName() << '\n';
    outfile->Close();
    std::cout << "==> " << APPNAME << " is done!\n";

    if (!gROOT->IsBatch()) {
        cv.GetResults()[0].DrawAvgROCCurve(kTRUE, "Avg ROC for BDT");
        cv.GetResults()[1].DrawAvgROCCurve(kTRUE, "Avg ROC for NN");
    }
}
