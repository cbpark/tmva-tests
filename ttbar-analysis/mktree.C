#include "TFile.h"
#include "TTree.h"

void mktree() {
    TFile f("ttbar_analysis.root", "recreate");

    TTree sig("TreeS", "signal (ttbar)");
    sig.ReadFile("./signal.dat", "mt2:mbl:mbbll");
    sig.Write();

    TTree bkg("TreeB", "background (WW + 2b)");
    bkg.ReadFile("./background.dat", "mt2:mbl:mbbll");
    bkg.Write();
}
