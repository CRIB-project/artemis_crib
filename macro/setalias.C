{
//   //=========================TEL PI=====================================
//   // tel1
//   tree->SetAlias("tel1_ene","(abs(sqrt(tel1.fdEY * tel1.fEtotal) - 5.3) < 0.5) && (abs(tel1.GetETimingvec(0) + 172.5) < 27.5)");
//   tree->SetAlias("tel1_p_timing","abs((tel1.fEtotal + 0.20*tel1.GetETimingvec(0)) + 15.5) < 5.5 && abs(tel1.GetETimingvec(0) + 170.0) < 30.0");
//   tree->SetAlias("tel1_timing","tel1.GetETimingvec(0) < -100.0");
//
//   // tel2
//   tree->SetAlias("tel2_ene","abs(sqrt(tel2.fdEY * tel2.fEtotal) - 5.45) < 0.55");
//   tree->SetAlias("tel2_p_timing","abs((tel2.fEtotal + 0.15*tel2.GetETimingvec(0)) + 6.5) < 6.5 && abs(tel2.GetETimingvec(0) + 170.0) < 30.0");
//   tree->SetAlias("tel2_timing","tel2.GetETimingvec(0) < -100.0");
//
//   // tel3
//   tree->SetAlias("tel3_ene","abs(sqrt(tel3.fdEY * tel3.fEtotal) - 7.7) < 1.0");
//   tree->SetAlias("tel3_p_timing","abs((tel3.fEtotal + 0.15*tel3.GetETimingvec(0)) + 6.5) < 6.5 && abs(tel3.GetETimingvec(0) + 180.0) < 40.0");
//   tree->SetAlias("tel3_timing","tel3.GetETimingvec(0) < -100.0");
//
//   // tel4
//   tree->SetAlias("tel4_ene","(abs(sqrt(tel4.fdE * tel4.fEtotal) - 2.5) < 1.0) || (tel4.fdE < 0.1)");
//   tree->SetAlias("tel4_p_timing","abs((tel4.fEtotal + 0.23*tel4.GetETimingvec(0)) + 23.0) < 8.0 && abs(tel4.GetETimingvec(0) + 190.0) < 30.0");
//   tree->SetAlias("tel4_timing","tel4.GetETimingvec(0) > -210 && tel4.GetETimingvec(0) < -180");
//
//   // tel5
//   tree->SetAlias("tel5_ene","(abs(sqrt(tel5.fdE * tel5.fEtotal) - 2.9) < 0.7) || (tel5.fdE < 0.1)");
//   tree->SetAlias("tel5_p_timing","abs((tel5.fEtotal + 0.23*tel5.GetETimingvec(0)) + 26.0) < 6.0 && abs(tel5.GetETimingvec(0) + 190.0) < 30.0");
//   tree->SetAlias("tel5_timing","tel5.GetETimingvec(0) > -215 && tel5.GetETimingvec(0) < -180");
//
//   //=========================BEAM ENERGY======================================
//   // general
   tree->SetAlias("ppaca_trig","abs(f3appac.fTAnode + 33.0) < 7.0");
   tree->SetAlias("ppacb_trig","abs(f3bppac.fTAnode) < 1.0");
   tree->SetAlias("trigger","ppaca_trig && ppacb_trig");
   tree->SetAlias("coin_trig","trigger && coin.fID==0");
//
//   // si26
   tree->SetAlias("si26_rf0_1","abs(rf_cal_0.fTiming - 26.0) < 4.0"); //19.5 < 29.5
   tree->SetAlias("si26_rf0_2","abs(rf_cal_0.fTiming - 77.5) < 4.0");
   tree->SetAlias("si26_tof","abs((f3bppac.fTAnode - f3appac.fTAnode) - 31.75) < 1.25"); // before 0249
   tree->SetAlias("si26","coin_trig && si26_tof && (si26_rf0_1 || si26_rf0_2)");
   //tree->SetAlias("si26","si26_rf0_1 || si26_rf0_2"); // for run0196
//
//   // al25
//   tree->SetAlias("al25_rf0_1","rf_cal_0.fTiming < 16 || rf_cal_0.fTiming > 110");
//   tree->SetAlias("al25_rf0_2","abs(rf_cal_0.fTiming - 63.0) < 4.5"); // 58 < 68
//   tree->SetAlias("al25_tof","abs((f3bppac.fTAnode - f3appac.fTAnode) - 32.5) < 1.5"); // 31.0 < 34.0
//   tree->SetAlias("al25","coin_trig && al25_tof && (al25_rf0_1 || al25_rf0_2)");
//   //tree->SetAlias("al25","al25_rf0_1 || al25_rf0_2"); // for run0196
//
//   // mg24
//   tree->SetAlias("mg24_rf0_1","abs(rf_cal_0.fTiming - 45.0) < 3.0"); // 42 < 48
//   tree->SetAlias("mg24_rf0_2","abs(rf_cal_0.fTiming - 97.0) < 3.0"); // 94 < 100
//   tree->SetAlias("mg24_tof","abs((f3bppac.fTAnode - f3appac.fTAnode) - 33.5) < 0.75"); // 33.0 < 34.5
//   tree->SetAlias("mg24","coin_trig && mg24_tof && (mg24_rf0_1 || mg24_rf0_2)");
//   //tree->SetAlias("mg24","mg24_rf0_1 || mg24_rf0_2"); // for run0196
//
   //=========================PHYSICS RUN======================================
   //// PPAC
   ////before 0249
   //tree->SetAlias("ppaca_trig","abs(f3appac.fTAnode + 33.0) < 5.0");
   ////after 0249
   //tree->SetAlias("ppaca_trig","abs(f3bppac.fTAnode) < 1.0");

   //tree->SetAlias("trigger","ppaca_trig && ppacb_trig");
   //tree->SetAlias("single_trig","trigger && single.fID==0");
   //tree->SetAlias("coin_trig","trigger && coin.fID==0");

   //// RF0 (coin tel1)
   //tree->SetAlias("si26_rf0_1","abs(rf_cal_0.fTiming - 26.0) < 4.5"); //19.5 < 29.5
   //tree->SetAlias("si26_rf0_2","abs(rf_cal_0.fTiming - 77.5) < 4.5");

   //// si26 tof (before 0249)
   //tree->SetAlias("si26_tof","abs((f3bppac.fTAnode - f3appac.fTAnode) - 31.75) < 1.25"); // before 0249
   ////tree->SetAlias("si26_tof","abs((f3bppac.fTAnode - f3appac.fTAnode) - 25.25) < 1.25"); // after 0249

   //// si26
   //tree->SetAlias("si26","si26_tof && (si26_rf0_1 || si26_rf0_2)");


  //=============================CALIBRATION RUN================================

   //********TEL 1*********
   //run 0273
   //tree->SetAlias("proton", "abs(tel1.GetEvec(0) - 7.2) < 0.8 && abs(tel1.fdEY - 1.2) < 0.4");
   //tree->SetAlias("alpha", "abs(tel1.fdEY - 23.5) < 0.5 && abs(tel1.fdEX - 1.25) < 0.75");

   //run 0275
   //tree->SetAlias("proton", "abs(tel1.GetEvec(0) - 7.25) < 0.75 && abs(tel1.fdEY - 2.65) < 0.35");
   //tree->SetAlias("alpha", "abs(tel1.fdEY - 8.0) < 0.5 && abs(tel1.fdEX - 2.35) < 0.65");

   //run 0277
   //tree->SetAlias("proton", "abs(tel1.GetEvec(0) - 14.25) < 0.75 && abs(tel1.fdEY - 1.8) < 0.5 && tel1.fXID>11");
   //tree->SetAlias("alpha", "abs(tel1.fdEY - 15.0) < 0.5 && abs(tel1.fdEX - 1.7) < 0.6");

   //run 0281
   //tree->SetAlias("proton", "abs(tel1.GetEvec(1) - 7.5) < 1.5 && abs(tel1.GetEvec(0) - 9.75) < 1.25");
   //tree->SetAlias("alpha", "abs(tel1.fdEY - 18.5) < 0.5 && abs(tel1.fdEX - 1.5) < 0.5");

   //run 0282
   //tree->SetAlias("proton", "abs(tel1.GetEvec(1) - 11.0) < 1.5 && abs(tel1.GetEvec(0) - 8.5) < 1.0");
   //tree->SetAlias("alpha", "abs(tel1.fdEY - 22.2) < 0.3 && abs(tel1.fdEX - 1.0) < 0.5");

   //run 0284
   // no proton?
   //tree->SetAlias("alpha", "abs(tel1.fdEY - 2.5) < 0.5 && abs(tel1.fdEX - 3.5) < 0.6");



   //********TEL 2*********
   //run 0288
   // no proton?
   //tree->SetAlias("alpha", "abs(tel2.fdEY - 3.5) < 1.0 && abs(tel2.fEtotal - 6.0) < 0.5");

   //run 0289
   //tree->SetAlias("proton", "abs(tel2.GetEvec(0) - 7.25) < 0.75 && abs(tel2.fdEY - 2.75) < 0.45");
   //tree->SetAlias("alpha", "abs(tel2.fdEY - 8.35) < 0.35 && abs(tel2.fdEX - 2.0) < 0.5");

   //run 0294
   //tree->SetAlias("proton", "abs(tel2.GetEvec(0) - 14.5) < 0.5 && abs(tel2.fdEY - 1.75) < 0.45");
   // no alpha

   //run 0296
   //tree->SetAlias("proton", "abs(tel2.GetEvec(1) - 7.25) < 1.25 && abs(tel2.GetEvec(0) - 10.5) < 1.0");
   //tree->SetAlias("alpha", "abs(tel2.fdEY - 18.6) < 0.3 && abs(tel2.fdEX - 1.25) < 0.25");



   //********TEL 3*********
   //tree->SetAlias("exclude_ch7","tel3.fYID!=7");

   //run 0287
   // no proton?
   //tree->SetAlias("alpha", "abs(tel3.fdEY - 3.5) < 1.0 && abs(tel3.fEtotal - 6.1) < 0.5 && exclude_ch7");

   //run 0290
   //tree->SetAlias("proton", "abs(tel3.GetEvec(0) - 4.95) < 0.55 && abs(tel3.fdEY - 5.0) < 0.5 && exclude_ch7");
   //tree->SetAlias("alpha", "abs(tel3.fdEY - 8.4) < 0.4 && abs(tel3.fdEX - 2.0) < 0.5 && exclude_ch7");

   //run 0292
   //tree->SetAlias("proton", "abs(tel3.GetEvec(0) - 12.15) < 0.45 && abs(tel3.fdEY - 3.05) < 0.35 && exclude_ch7");
   // no alpha

   //run 0295
   //tree->SetAlias("proton", "abs(tel3.GetEvec(1) - 5.5) < 1.5 && abs(tel3.GetEvec(0) - 10.15) < 1.15 && exclude_ch7");
   //tree->SetAlias("alpha", "abs(tel3.fdEY - 18.75) < 0.75 && abs(tel3.fdEX - 1.05) < 0.55 && exclude_ch7");


   //********TEL 4*********
   //run 0297
   //tree->SetAlias("proton", "abs(tel4.GetEvec(1) - 9.5) < 1.0 && abs(tel4.GetEvec(0) - 8.5) < 1.5");
   //tree->SetAlias("alpha", "abs(tel4.GetEvec(0) - 15.25) < 0.75 && abs(tel4.fdE - 4.2) < 0.6");

   //run 0299
   //tree->SetAlias("proton", "abs(tel4.GetEvec(0) - 9.4) < 0.3");
   //tree->SetAlias("alpha", "abs(tel4.fdE - 10.2) < 0.1");

   //run 0302
   //tree->SetAlias("proton", "abs(tel4.GetEvec(0) - 11.5) < 0.2");
   //tree->SetAlias("alpha", "abs(tel4.GetEvec(0) - 5.25) < 1.25 && abs(tel4.fdE - 7.0) < 1.0");

   //run 0304
   //tree->SetAlias("proton", "abs(tel4.GetEvec(0) - 5.1) < 0.2");
   //tree->SetAlias("alpha", "abs(tel4.fdE - 5.05) < 0.2");

   //run 0306
   //tree->SetAlias("proton", "abs(tel4.GetEvec(0) - 7.2) < 0.2");
   //tree->SetAlias("alpha", "abs(tel4.fdE - 7.05) < 0.2");


   //********TEL 5*********
   //run 0298
   //tree->SetAlias("proton", "abs(tel5.GetEvec(1) - 9.25) < 0.75 && abs(tel5.GetEvec(0) - 7.35) < 0.75");
   //tree->SetAlias("alpha", "abs(tel5.GetEvec(0) - 15.0) < 0.5 && abs(tel5.fdE - 4.5) < 0.5");

   //run 0300
   //tree->SetAlias("proton", "abs(tel5.GetEvec(0) - 9.4) < 0.3 && abs(tel5.fdE - 0.6) < 0.4");
   //tree->SetAlias("alpha", "abs(tel5.fdE - 9.65) < 0.15");

   //run 0301
   //tree->SetAlias("proton", "abs(tel5.GetEvec(0) - 11.6) < 0.6 && abs(tel5.fdE - 0.5) < 0.3");
   //tree->SetAlias("alpha", "abs(tel5.GetEvec(0) - 6.0) < 2.0 && abs(tel5.fEtotal - 12.0) < 1.0");

   //run 0303
   //tree->SetAlias("proton", "abs(tel5.GetEvec(0) - 13.3) < 0.3 && abs(tel5.fdE - 0.6) < 0.2");
   //tree->SetAlias("alpha", "abs(tel5.GetEvec(0) - 8.5) < 1.5 && abs(tel5.fEtotal - 14.0) < 1.0");

   //run 0304
   //tree->SetAlias("proton", "abs(tel5.GetEvec(0) - 5.1) < 0.3 && abs(tel5.fdE - 1.0) < 0.2");
   //tree->SetAlias("alpha", "abs(tel5.fdE - 5.1) < 0.2")

   //run 0305
   //tree->SetAlias("proton", "abs(tel5.GetEvec(0) - 7.2) < 0.3 && abs(tel5.fdE - 0.9) < 0.3");
   //tree->SetAlias("alpha", "abs(tel5.fdE - 7.05) < 0.15")

   //============================================================================

}
