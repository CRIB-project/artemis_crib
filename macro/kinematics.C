/////////////////////////////////////////////////////////////////////////////
// kinematic TGraph object (like LISE calculator)
// A(a,b)B reaction
// input:
//    Ebeam                  : beam energy (MeV)
//    beam_z, beam_a         : (Z, A) for particle "a"
//    target_z, target_a     : (Z, A) for particle "A"
//    fragment_z, fragment_a : (Z, A) for particle "b"
//    residual_z, residual_a : (Z, A) for particle "B"
//    particle               : 0: fragment particle, 1: residual particle
/////////////////////////////////////////////////////////////////////////////

// angle lab vs. angle cm
TGraph *GetALabACM(Double_t Ebeam,
                   Int_t beam_z, Int_t beam_a, Int_t target_z, Int_t target_a,
                   Int_t fragment_z, Int_t fragment_a, Int_t residual_z, Int_t residual_a,
                   UShort_t particle = 0) {
    if (particle != 0 && particle != 1) {
        std::cerr << "input error: last argument should be 0 or 1" << std::endl;
        return NULL;
    }

    // get mass MeV unit
    Double_t Ma = gAtomicMassTable->GetNucleusMass(beam_z, beam_a);
    Double_t MA = gAtomicMassTable->GetNucleusMass(target_z, target_a);
    Double_t Mb = gAtomicMassTable->GetNucleusMass(fragment_z, fragment_a);
    Double_t MB = gAtomicMassTable->GetNucleusMass(residual_z, residual_a);

    // prepare initial particles
    TLorentzVector beam_vec(0., 0., 0., Ma);
    Double_t beta_beam = TMath::Sqrt(1.0 - TMath::Power(Ma / (Ma + Ebeam), 2));
    beam_vec.Boost(0., 0., beta_beam);
    TLorentzVector target_vec(0., 0., 0., MA);
    TLorentzVector compound_vec = beam_vec + target_vec;
    TVector3 beta_vec = compound_vec.BoostVector();
    beam_vec.Boost(-beta_vec);
    target_vec.Boost(-beta_vec);
    Double_t Ecm = beam_vec.E() + target_vec.E();

    // prepare reaction particles
    Double_t Ebcm = (Ecm + (Mb * Mb - MB * MB) / Ecm) / 2.0;
    Double_t pbcm = TMath::Sqrt(Ebcm * Ebcm - Mb * Mb);
    TLorentzVector fragment_vec(0., 0., pbcm, Mb);
    TLorentzVector residual_vec(0., 0., -pbcm, MB);

    TGraph *gr = new TGraph();
    for (Int_t i = 0; i <= 180; i++) {
        Double_t theta_cm = (Double_t)i * TMath::DegToRad();
        TLorentzVector frag_cp = fragment_vec;
        TLorentzVector resi_cp = residual_vec;
        frag_cp.RotateY(theta_cm);
        resi_cp.RotateY(theta_cm);
        frag_cp.Boost(beta_vec);
        resi_cp.Boost(beta_vec);

        // fragment information
        Double_t b_angle_lab = frag_cp.Theta() * TMath::RadToDeg();

        // residual information
        Double_t B_angle_lab = resi_cp.Theta() * TMath::RadToDeg();

        if (particle == 0) {
            gr->SetPoint(i, (Double_t)i, b_angle_lab);
        } else {
            gr->SetPoint(i, (Double_t)i, B_angle_lab);
        }
    }

    if (particle == 0) {
        gr->SetTitle(Form("ALabACM (Z, A)=(%d, %d);Angle CM(deg);Angle Lab(deg)", fragment_z, fragment_a));
    } else {
        gr->SetTitle(Form("ALabACM (Z, A)=(%d, %d);Angle CM(deg);Angle Lab(deg)", residual_z, residual_a));
    }
    gr->SetLineColor(kRed);
    gr->SetLineWidth(2);
    gr->SetMarkerColor(kRed);
    gr->SetMarkerSize(2);
    gr->SetMarkerStyle(8);

    return gr;
}

// energy lab vs. angle cm
TGraph *GetELabACM(Double_t Ebeam,
                   Int_t beam_z, Int_t beam_a, Int_t target_z, Int_t target_a,
                   Int_t fragment_z, Int_t fragment_a, Int_t residual_z, Int_t residual_a,
                   UShort_t particle = 0) {
    if (particle != 0 && particle != 1) {
        std::cerr << "input error: last argument should be 0 or 1" << std::endl;
        return NULL;
    }
    // get mass MeV unit
    Double_t Ma = gAtomicMassTable->GetNucleusMass(beam_z, beam_a);
    Double_t MA = gAtomicMassTable->GetNucleusMass(target_z, target_a);
    Double_t Mb = gAtomicMassTable->GetNucleusMass(fragment_z, fragment_a);
    Double_t MB = gAtomicMassTable->GetNucleusMass(residual_z, residual_a);

    // prepare initial particles
    TLorentzVector beam_vec(0., 0., 0., Ma);
    Double_t beta_beam = TMath::Sqrt(1.0 - TMath::Power(Ma / (Ma + Ebeam), 2));
    beam_vec.Boost(0., 0., beta_beam);
    TLorentzVector target_vec(0., 0., 0., MA);
    TLorentzVector compound_vec = beam_vec + target_vec;
    TVector3 beta_vec = compound_vec.BoostVector();
    beam_vec.Boost(-beta_vec);
    target_vec.Boost(-beta_vec);
    Double_t Ecm = beam_vec.E() + target_vec.E();

    // prepare reaction particles
    Double_t Ebcm = (Ecm + (Mb * Mb - MB * MB) / Ecm) / 2.0;
    Double_t pbcm = TMath::Sqrt(Ebcm * Ebcm - Mb * Mb);
    TLorentzVector fragment_vec(0., 0., pbcm, Mb);
    TLorentzVector residual_vec(0., 0., -pbcm, MB);

    TGraph *gr = new TGraph();
    for (Int_t i = 0; i <= 180; i++) {
        Double_t theta_cm = (Double_t)i * TMath::DegToRad();
        TLorentzVector frag_cp = fragment_vec;
        TLorentzVector resi_cp = residual_vec;
        frag_cp.RotateY(theta_cm);
        resi_cp.RotateY(theta_cm);
        frag_cp.Boost(beta_vec);
        resi_cp.Boost(beta_vec);

        // fragment information
        Double_t b_E_lab = frag_cp.E() - frag_cp.M();

        // residual information
        Double_t B_E_lab = resi_cp.E() - resi_cp.M();

        if (particle == 0) {
            gr->SetPoint(i, (Double_t)i, b_E_lab);
        } else {
            gr->SetPoint(i, (Double_t)i, B_E_lab);
        }
    }

    if (particle == 0) {
        gr->SetTitle(Form("ELabACM (Z, A)=(%d, %d);Angle CM(deg);Angle Lab(deg)", fragment_z, fragment_a));
    } else {
        gr->SetTitle(Form("ELabACM (Z, A)=(%d, %d);Angle CM(deg);Angle Lab(deg)", residual_z, residual_a));
    }
    gr->SetLineColor(kRed);
    gr->SetLineWidth(2);
    gr->SetMarkerColor(kRed);
    gr->SetMarkerSize(2);
    gr->SetMarkerStyle(8);

    return gr;
}

// angle lab of "B" vs. angle lab of "b"
TGraph *GetALabALab(Double_t Ebeam,
                    Int_t beam_z, Int_t beam_a, Int_t target_z, Int_t target_a,
                    Int_t fragment_z, Int_t fragment_a, Int_t residual_z, Int_t residual_a) {
    // get mass MeV unit
    Double_t Ma = gAtomicMassTable->GetNucleusMass(beam_z, beam_a);
    Double_t MA = gAtomicMassTable->GetNucleusMass(target_z, target_a);
    Double_t Mb = gAtomicMassTable->GetNucleusMass(fragment_z, fragment_a);
    Double_t MB = gAtomicMassTable->GetNucleusMass(residual_z, residual_a);

    // prepare initial particles
    TLorentzVector beam_vec(0., 0., 0., Ma);
    Double_t beta_beam = TMath::Sqrt(1.0 - TMath::Power(Ma / (Ma + Ebeam), 2));
    beam_vec.Boost(0., 0., beta_beam);
    TLorentzVector target_vec(0., 0., 0., MA);
    TLorentzVector compound_vec = beam_vec + target_vec;
    TVector3 beta_vec = compound_vec.BoostVector();
    beam_vec.Boost(-beta_vec);
    target_vec.Boost(-beta_vec);
    Double_t Ecm = beam_vec.E() + target_vec.E();

    // prepare reaction particles
    Double_t Ebcm = (Ecm + (Mb * Mb - MB * MB) / Ecm) / 2.0;
    Double_t pbcm = TMath::Sqrt(Ebcm * Ebcm - Mb * Mb);
    TLorentzVector fragment_vec(0., 0., pbcm, Mb);
    TLorentzVector residual_vec(0., 0., -pbcm, MB);

    TGraph *gr = new TGraph();
    for (Int_t i = 0; i <= 180; i++) {
        Double_t theta_cm = (Double_t)i * TMath::DegToRad();
        TLorentzVector frag_cp = fragment_vec;
        TLorentzVector resi_cp = residual_vec;
        frag_cp.RotateY(theta_cm);
        resi_cp.RotateY(theta_cm);
        frag_cp.Boost(beta_vec);
        resi_cp.Boost(beta_vec);

        // fragment information
        Double_t b_angle_lab = frag_cp.Theta() * TMath::RadToDeg();

        // residual information
        Double_t B_angle_lab = resi_cp.Theta() * TMath::RadToDeg();

        gr->SetPoint(i, b_angle_lab, B_angle_lab);
    }

    gr->SetTitle(Form("ALabALab;Angle CM(deg);Angle Lab(deg)"));
    gr->SetLineColor(kRed);
    gr->SetLineWidth(2);
    gr->SetMarkerColor(kRed);
    gr->SetMarkerSize(2);
    gr->SetMarkerStyle(8);

    return gr;
}

// energy lab vs. angle lab
TGraph *GetELabALab(Double_t Ebeam,
                    Int_t beam_z, Int_t beam_a, Int_t target_z, Int_t target_a,
                    Int_t fragment_z, Int_t fragment_a, Int_t residual_z, Int_t residual_a,
                    UShort_t particle = 0) {
    if (particle != 0 && particle != 1) {
        std::cerr << "input error: last argument should be 0 or 1" << std::endl;
        return NULL;
    }
    // get mass MeV unit
    Double_t Ma = gAtomicMassTable->GetNucleusMass(beam_z, beam_a);
    Double_t MA = gAtomicMassTable->GetNucleusMass(target_z, target_a);
    Double_t Mb = gAtomicMassTable->GetNucleusMass(fragment_z, fragment_a);
    Double_t MB = gAtomicMassTable->GetNucleusMass(residual_z, residual_a);

    // prepare initial particles
    TLorentzVector beam_vec(0., 0., 0., Ma);
    Double_t beta_beam = TMath::Sqrt(1.0 - TMath::Power(Ma / (Ma + Ebeam), 2));
    beam_vec.Boost(0., 0., beta_beam);
    TLorentzVector target_vec(0., 0., 0., MA);
    TLorentzVector compound_vec = beam_vec + target_vec;
    TVector3 beta_vec = compound_vec.BoostVector();
    beam_vec.Boost(-beta_vec);
    target_vec.Boost(-beta_vec);
    Double_t Ecm = beam_vec.E() + target_vec.E();

    // prepare reaction particles
    Double_t Ebcm = (Ecm + (Mb * Mb - MB * MB) / Ecm) / 2.0;
    Double_t pbcm = TMath::Sqrt(Ebcm * Ebcm - Mb * Mb);
    TLorentzVector fragment_vec(0., 0., pbcm, Mb);
    TLorentzVector residual_vec(0., 0., -pbcm, MB);

    TGraph *gr = new TGraph();
    for (Int_t i = 0; i <= 180; i++) {
        Double_t theta_cm = (Double_t)i * TMath::DegToRad();
        TLorentzVector frag_cp = fragment_vec;
        TLorentzVector resi_cp = residual_vec;
        frag_cp.RotateY(theta_cm);
        resi_cp.RotateY(theta_cm);
        frag_cp.Boost(beta_vec);
        resi_cp.Boost(beta_vec);

        // fragment information
        Double_t b_angle_lab = frag_cp.Theta() * TMath::RadToDeg();
        Double_t b_E_lab = frag_cp.E() - frag_cp.M();

        // residual information
        Double_t B_angle_lab = resi_cp.Theta() * TMath::RadToDeg();
        Double_t B_E_lab = resi_cp.E() - resi_cp.M();

        if (particle == 0) {
            gr->SetPoint(i, b_angle_lab, b_E_lab);
        } else {
            gr->SetPoint(i, B_angle_lab, B_E_lab);
        }
    }

    if (particle == 0) {
        gr->SetTitle(Form("ELabALab (Z, A)=(%d, %d);Angle CM(deg);Angle Lab(deg)", fragment_z, fragment_a));
    } else {
        gr->SetTitle(Form("ELabALab (Z, A)=(%d, %d);Angle CM(deg);Angle Lab(deg)", residual_z, residual_a));
    }
    gr->SetLineColor(kRed);
    gr->SetLineWidth(2);
    gr->SetMarkerColor(kRed);
    gr->SetMarkerSize(2);
    gr->SetMarkerStyle(8);

    return gr;
}
