#include "TFile.h"
#include "TTree.h"

const char *VARS = "mt2:mbl:mbbll";

void mktree() {
    TFile f("ttbar_analysis.root", "recreate");

    TTree sig("TreeS", "signal (ttbar)");
    sig.ReadFile("./signal.dat", VARS);
    sig.Write();

    TTree bkg("TreeB", "background (WW + 2b)");
    bkg.ReadFile("./background.dat", VARS);
    bkg.Write();
}
