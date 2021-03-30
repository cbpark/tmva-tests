#include <iostream>
#include <memory>
#include <string>
#include "TFile.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Factory.h"
#include "TMVA/Types.h"
#include "TTree.h"

const std::string APPNAME = "ttbarClassification";

void ttbarClassification() {
    std::cout << "==> Start " << APPNAME << '\n';

    TFile input{"ttbar-analysis/ttbar_analysis.root"};
    std::cout << "--- " << APPNAME
              << "  : Using input file: " << input.GetName() << '\n';

    auto dataloader = std::make_shared<TMVA::DataLoader>("dataset");
    dataloader->AddVariable("mt2", "M_{T2}", "", 'F');
    dataloader->AddVariable("mbl", "m_{bl}", "", 'F');
    dataloader->AddVariable("mbbll", "m_{bbll}", "", 'F');

    auto sig_tree = dynamic_cast<TTree *>(input.Get("TreeS"));
    dataloader->AddSignalTree(sig_tree, 1.0);
    auto bkg_tree = dynamic_cast<TTree *>(input.Get("TreeB"));
    dataloader->AddBackgroundTree(bkg_tree, 1.0);

    dataloader->PrepareTrainingAndTestTree("", "SplitMode=random:!V");

    auto outfile = std::make_unique<TFile>("ttbar_tmva.root", "recreate");
    TMVA::Factory factory{APPNAME, outfile.get(),
                          "!V:!Silent:Color:DrawProgressBar:Transformations="
                          "I;D;P;G,D:AnalysisType=Classification"};

    // factory.BookMethod(
    //     dataloader.get(), TMVA::Types::kCuts, "Cuts",
    //     "!H:!V:FitMethod=MC:EffSel:SampleSize=200000:VarProp=FSmart");

    factory.BookMethod(
        dataloader.get(), TMVA::Types::kBDT, "BDT",
        "!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:"
        "AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:"
        "SeparationType=GiniIndex:nCuts=20");

    factory.BookMethod(dataloader.get(), TMVA::Types::kMLP, "MLP",
                       "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:"
                       "HiddenLayers=N+1,N:TestRate=5:!UseRegulator");

    // factory.BookMethod(dataloader.get(), TMVA::Types::kSVM, "SVM",
    //                    "Gamma=0.25:Tol=0.001:VarTransform=Norm");

    // Train MVAs using the set of training events
    factory.TrainAllMethods();

    // Evaluate all MVAs using the set of test events
    factory.TestAllMethods();

    // Evaluate and compare performance of all configured MVAs
    factory.EvaluateAllMethods();

    std::cout << "==> Wrote root file: " << outfile->GetName() << '\n';
    std::cout << "==> " << APPNAME << " is done!\n";
}
