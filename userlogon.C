{
    // load user function
    gROOT->ProcessLine(".L macro/load_tcutg.C");
    gROOT->ProcessLine(".L macro/kinematics.C");
    // gROOT->ProcessLine(".L macro/dE_E_plot.C");

    // User commands register
    // cf definition : TCatCmdFactory *cf = TCatCmdFactory::Instance();
    cf->Register(art::crib::TCatCmdLoopStart::Instance());
    cf->Register(art::crib::TCatCmdLoopStop::Instance());
    cf->Register(new art::crib::TCmdXfitg);
    cf->Register(new art::crib::TCmdXstatus);
    cf->Register(new art::crib::TCmdXYblow);
    cf->Register(new art::crib::TCmdXblow);
    cf->Register(art::crib::TCatCmdTCutG::Instance());
    cf->Register(new art::crib::TCmdErase);
    cf->Register(new art::crib::TCmdDraw);

    // User decoder register
    // df definition: art::TModuleDecoderFactory *df = art::TModuleDecoderFactory::Instance();
    gInterpreter->ProcessLine("art::TModuleDecoderFactory::Instance()->Register(new art::crib::TModuleDecoderTimestamp);");

    // TCutG object (path to rootfile, tcutg object name)
    getAllTCutObject();

    // Http server
    // auto serv = new THttpServer("http:8080");
    // serv->SetItemField("/", "_monitoring", "100"); // monitoring interval in ms

    // TTree merge setting
    TTree::SetMaxTreeSize(1000000000000LL); // 1TB
}
