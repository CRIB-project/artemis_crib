/**
 * @file    TScalerMonitorProcessor.cc
 * @brief
 * @author  Kodai Okawa<okawa@cns.s.u-tokyo.ac.jp>
 * @date    2024-01-28 14:26:03
 * @note
 */

#include "TScalerMonitorProcessor.h"

#include <TAxis.h>
#include <TLegend.h>
#include <TStyle.h>
#include <chrono>

using art::crib::TScalerMonitorProcessor;

ClassImp(TScalerMonitorProcessor);

TScalerMonitorProcessor::TScalerMonitorProcessor()
    : fShortCount(0), fLongCount(0) {
    RegisterInputCollection("InputCollection",
                            "Scaler object inheriting from art::TScalerData",
                            fInputColName, TString("scatot"));

    RegisterProcessorParameter("ShortInterval", "the interval time (seconds) for short time monitor",
                               fShortInterval, 0);
    RegisterProcessorParameter("ShortDuration", "the time range (seconds) for short time monitor",
                               fShortDuration, 0);
    RegisterProcessorParameter("LongInterval", "the interval time (seconds) for long time monitor",
                               fLongInterval, 0);
    RegisterProcessorParameter("LongDuration", "the time range (seconds) for long time monitor",
                               fLongDuration, 0);
    IntVec_t ivec;
    RegisterProcessorParameter("Clock", "the channel number of 10Hz clock [ch, Hz]",
                               fClock, ivec);

    StringVec_t init_v;
    RegisterProcessorParameter("Channels", "the time range (seconds) for long time monitor",
                               fChannels, init_v);

    for (Int_t i = 0; i < SCALER_CH; i++) {
        fFactors[i] = 1.0;
        fGr_s[i] = nullptr;
        fGr_l[i] = nullptr;
    }
}

TScalerMonitorProcessor::~TScalerMonitorProcessor() {
    for (Int_t i = 0; i < SCALER_CH; i++) {
        delete fGr_s[i];
        delete fGr_l[i];
    }

    // delete main;
    // delete pad_s;
    // delete pad_l;
}

void TScalerMonitorProcessor::Init(TEventCollection *col) {
    gStyle->SetTimeOffset(-788918400); // to UNIX time
    gStyle->SetNdivisions(505);
    gStyle->SetPadGridX(1);
    gStyle->SetPadGridY(1);

    Info("Init", "Scaler live time chart");
    Info("Init", "NEED: analysis speed > trigger rate");
    Info("Init", "\tshort time chart: interval %d s, time range %d s", fShortInterval, fShortDuration);
    Info("Init", "\tlong time chart : interval %d s, time range %d s", fLongInterval, fLongDuration);
    Info("Init", "Display: channel, name, scale factor");
    for (Size_t i = 0; i < fChannels.size(); i++) {
        Info("Init", "Display: %s", fChannels[i].Data());
    }

    fInData = reinterpret_cast<TScalerData **>(col->GetObjectRef(fInputColName.Data()));
    if (!fInData) {
        SetStateError(TString::Format("input not found: %s", fInputColName.Data()));
        return;
    }

    for (Size_t i = 0; i < fChannels.size(); i++) {
        Int_t first = fChannels[i].First(',');
        Int_t last = fChannels[i].Last(',');
        Int_t len = fChannels[i].Length();
        if (first == last) {
            SetStateError("Input steering yaml is wrong, need \"ch, name, factor\"");
            return;
        }
        TString channel_str = fChannels[i](0, first);
        Int_t channel = channel_str.Remove(TString::kBoth, ' ').Atoi();
        fDisplayCh.emplace_back(channel);

        TString name = fChannels[i](first + 1, last - first - 1);
        fNames.emplace_back(name.Remove(TString::kBoth, ' '));

        TString factor_str = fChannels[i](last + 1, len - last - 1);
        Double_t factor = factor_str.Remove(TString::kBoth, ' ').Atof();

        fFactors[channel] = factor;
    }

    for (Int_t i = 0; i < SCALER_CH; i++) {
        fGr_s[i] = new TGraph();
        fGr_s[i]->SetLineWidth(1);
        fGr_s[i]->SetMarkerStyle(8);
        fGr_s[i]->SetMarkerSize(0.8);
        fGr_s[i]->SetMinimum(0.);
        fGr_s[i]->SetMaximum(1000.);

        fGr_l[i] = new TGraph();
        fGr_l[i]->SetLineWidth(1);
        fGr_l[i]->SetMarkerStyle(8);
        fGr_l[i]->SetMarkerSize(0.8);
        fGr_l[i]->SetMinimum(0.);
        fGr_l[i]->SetMaximum(1000.);
    }

    for (Size_t i = 0; i < fChannels.size(); i++) {
        fGr_s[fDisplayCh[i]]->SetTitle("count rate (short);;cps");
        fGr_s[fDisplayCh[i]]->SetName(fNames[i]);
        fGr_s[fDisplayCh[i]]->SetLineColor(fColor_list[i]);
        fGr_s[fDisplayCh[i]]->SetMarkerColor(fColor_list[i]);
        fGr_s[fDisplayCh[i]]->GetXaxis()->SetTimeDisplay(1);
        fGr_s[fDisplayCh[i]]->GetXaxis()->SetTimeFormat("%m/%d %H:%M");

        fGr_l[fDisplayCh[i]]->SetTitle("count rate (long);;cps");
        fGr_l[fDisplayCh[i]]->SetName(fNames[i]);
        fGr_l[fDisplayCh[i]]->SetLineColor(fColor_list[i]);
        fGr_l[fDisplayCh[i]]->SetMarkerColor(fColor_list[i]);
        fGr_l[fDisplayCh[i]]->GetXaxis()->SetTimeDisplay(1);
        fGr_l[fDisplayCh[i]]->GetXaxis()->SetTimeFormat("%m/%d %H:%M");
    }

    main = new TCanvas("canvas", "monitor", 800, 800);
    pad_s = new TPad("short", "short", 0.0, 0.5, 1.0, 1.0);
    pad_l = new TPad("long", "long", 0.0, 0.0, 1.0, 0.5);
    pad_s->Draw();
    pad_l->Draw();

    fIsFirst = true;
}

void TScalerMonitorProcessor::Process() {
    Long_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if (fIsFirst) {
        fStartShort = now;
        fStartLong = now;
        fCurrentShort = now;
        fCurrentLong = now;

        fIsFirst = false;

        const TScalerData *const scadata = static_cast<TScalerData *>(*fInData);
        for (Int_t i = 0; i < SCALER_CH; i++) {
            fShortScatot[i] = scadata->GetValue(i);
            fShortScadiff[i] = scadata->GetValue(i);
            fLongScatot[i] = scadata->GetValue(i);
            fLongScadiff[i] = scadata->GetValue(i);
        }
    }

    ProcessShort(now);
    ProcessLong(now);
}

void TScalerMonitorProcessor::ProcessShort(Long_t now) {
    if (now - fCurrentShort > fShortInterval) {
        const TScalerData *const scadata = static_cast<TScalerData *>(*fInData);
        if (scadata->GetValue(fClock[0]) - fShortScatot[fClock[0]] <= 0) {
            for (Int_t i = 0; i < SCALER_CH; i++) {
                fShortScatot[i] = scadata->GetValue(i);
                fShortScadiff[i] = scadata->GetValue(i);
            }
            fCurrentShort = now;
            return;
        }

        for (Int_t i = 0; i < SCALER_CH; i++) {
            fShortScadiff[i] = scadata->GetValue(i) - fShortScatot[i];
            fShortScatot[i] = scadata->GetValue(i);
        }

        for (Int_t i = 0; i < SCALER_CH; i++) {
            fShortScadiff[i] = (UInt_t)(fClock[1] * fShortScadiff[i] / fShortScadiff[fClock[0]]);
            fGr_s[i]->SetPoint(fShortCount, now, fShortScadiff[i] * fFactors[i]);

            if (now - fStartShort > fShortDuration) { // slide
                for (Int_t j = 0; j < fShortCount; j++) {
                    Double_t x, y;
                    fGr_s[i]->GetPoint(j + 1, x, y);
                    fGr_s[i]->SetPoint(j, x, y);
                }
                fGr_s[i]->RemovePoint(fShortCount);
            }
        }

        // draw process
        pad_s->cd();
        Bool_t isfirst_obj = true;
        TLegend *legend = new TLegend(0.78, 0.70, 0.95, 0.95);
        for (Size_t i = 0; i < fDisplayCh.size(); i++) {
            if (isfirst_obj) {
                fGr_s[fDisplayCh[i]]->Draw("apl");
                legend->AddEntry(fGr_s[fDisplayCh[i]], fNames[i], "pl");
                isfirst_obj = false;
                continue;
            }
            fGr_s[fDisplayCh[i]]->Draw("pl");
            legend->AddEntry(fGr_s[fDisplayCh[i]], fNames[i], "pl");
        }
        legend->Draw();
        main->Update();

        if (now - fStartShort <= fShortDuration) {
            fShortCount++;
        }
        fCurrentShort = now;
    }
}

void TScalerMonitorProcessor::ProcessLong(Long_t now) {
    if (now - fCurrentLong > fLongInterval) {
        const TScalerData *const scadata = static_cast<TScalerData *>(*fInData);
        if (scadata->GetValue(fClock[0]) - fLongScatot[fClock[0]] <= 0) {
            for (Int_t i = 0; i < SCALER_CH; i++) {
                fLongScatot[i] = scadata->GetValue(i);
                fLongScadiff[i] = scadata->GetValue(i);
            }
            fCurrentLong = now;
            return;
        }
        for (Int_t i = 0; i < SCALER_CH; i++) {
            fLongScadiff[i] = scadata->GetValue(i) - fLongScatot[i];
            fLongScatot[i] = scadata->GetValue(i);
        }

        for (Int_t i = 0; i < SCALER_CH; i++) {
            fLongScadiff[i] = (UInt_t)(fClock[1] * fLongScadiff[i] / fLongScadiff[fClock[0]]);
            fGr_l[i]->SetPoint(fLongCount, now, fLongScadiff[i] * fFactors[i]);

            if (now - fStartLong > fLongDuration) { // slide
                for (Int_t j = 0; j < fLongCount; j++) {
                    Double_t x, y;
                    fGr_l[i]->GetPoint(j + 1, x, y);
                    fGr_l[i]->SetPoint(j, x, y);
                }
                fGr_l[i]->RemovePoint(fLongCount);
            }
        }

        // draw process
        pad_l->cd();
        Bool_t isfirst_obj = true;
        for (Int_t &ch : fDisplayCh) {
            if (isfirst_obj) {
                fGr_l[ch]->Draw("apl");
                isfirst_obj = false;
                continue;
            }
            fGr_l[ch]->Draw("pl");
        }
        main->Update();

        if (now - fStartLong <= fLongDuration) {
            fLongCount++;
        }
        fCurrentLong = now;
    }
}
