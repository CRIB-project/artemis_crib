/**
 * @file    TTGTIKProcessor.h
 * @brief   Processor for reconstructing reaction positions using the Thick Gas Target Inverse Kinematics (TGTIK) method.
 * @author  Kodai Okawa <okawa@cns.s.u-tokyo.ac.jp>
 * @date    2023-08-01 11:11:02
 * @note    last modified: 2025-03-05 16:12:38
 * @details
 */

#ifndef CRIB_TTGTIKPROCESSOR_H_
#define CRIB_TTGTIKPROCESSOR_H_

#include "../telescope/TTelescopeData.h"
#include <TProcessor.h>
#include <TSrim.h> // TSrim library
#include <TTrack.h>

class TClonesArray;

namespace art::crib {

/**
 *
 * @class TTGTIKProcessor
 * @brief Calculates the reaction position using telescope and tracking data.
 *
 * This class read two input data from a TClonesArray("art::crib::TTelescopeData")
 * and a TClonesArray("art::TTrack").
 * It applies Thick Gas Target Method and get reaction position and Ecm.
 *
 * ### Example Steering File
 *
 * ```yaml
 * Processor:
 *   - name: MyTTGTIKProcessor
 *     type: art::crib::TTGTIKProcessor
 *     parameter:
 *       DetectorParameter: prm_detectors  # [TString] Name of the telescope parameter collection (detector parameters)
 *       ExcitedEnergy: -1  # [Double_t] Excited state energy (MeV); use a negative value if not applicable
 *       InitialBeamEnergy: 0  # [Double_t] Initial beam energy (in MeV) immediately after the exit window
 *       InputCollection: tel  # [TString] Input collection of telescope data objects (derived from TTelescopeData)
 *       InputTrackCollection: track  # [TString] Input collection of tracking data objects (derived from TTrack)
 *       OutputCollection: result  # [TString] Output collection containing reaction reconstruction information using the TGTIK method
 *       OutputTransparency: 0  # [Bool_t] Output is persistent if false (default)
 *       ParticleAArray: []  # [IntVec_t] Array of mass numbers (A) for reaction particles
 *       ParticleZArray: []  # [IntVec_t] Array of atomic numbers (Z) for reaction particles
 *       TargetName: ""  # [TString] Name of the target gas (used in TSrim calculation)
 *       TargetParameter: prm_targets  # [TString] Name of the target parameter collection
 *       TargetPressure: 0  # [Double_t] Target gas pressure in Torr
 *       TargetTemperature: 0  # [Double_t] Target gas temperature in Kelvin
 *       UseCenterPosition: 0  # [Bool_t] Flag to use the detector's center position (useful when the DSSSD is not operational)
 *       UseCustomFunction: 0  # [Bool_t] Flag to enable custom processing functions for additional corrections
 *       Verbose: 1  # [Int_t] verbose level (default 1 : non quiet)
 * ```
 *
 * \warning `UseCustomFunction` is designed for specific analysis, currently for 26Si(a, p) analysis.
 *
 * ### Kinematics Calculation
 *
 * This processor is using classical (non-relativistic) kinematics.
 * Okawa note the relationship. See my master thesis (Japanese) for the details.
 *
 * Consider Ion1 + Ion2 -> Ion3 + Ion4 reaction, that is, Ion2(Ion1, Ion3)Ion4 reaction.
 *
 * - \f$ M \f$: mass
 * - \f$ v \f$: velocity
 * - \f$ v_2 = 0 \f$: for the target ion
 * - \f$ {}_{\mathrm{CM}} \f$: center-of-mass system
 * - \f$ \theta\f$: scatter angle of Ion3
 * - \f$ Q \f$: Q-value
 *
 * Based on the relationship between the center-of-mass frame and the laboratory frame,
 *
 * \f[
 * \begin{align}
 * v_{\mathrm{CM}} &= \frac{M_2}{M_1 + M_2} v_1 \\
 * v_{1\mathrm{CM}} &= v_1 - v_{\mathrm{CM}} = \frac{M_2}{M_1}v_{\mathrm{CM}} \\
 * v_{2\mathrm{CM}} &= 0 - v_{\mathrm{CM}} = -v_{\mathrm{CM}}
 * \end{align}
 * \f]
 *
 * The speed of the center of mass, \f$ v_{\mathrm{CM}} \f$, and kinetic energy, \f$ E_{\mathrm{CM}} \f$
 * can be expressed as follows based on the definition of the center of mass.
 *
 * \f[
 * \begin{align}
 * E_{\mathrm{CM}} &= \frac{M_2(M_1 + M_2)}{2M_1} v_{\mathrm{CM}}^2 \\
 * &= \alpha v_{\mathrm{CM}}^2
 * \end{align}
 * \f]
 *
 * Here, \f$ \alpha \f$ is defined as:
 *
 * \f[
 * \begin{align}
 * \alpha = \frac{M_2(M_1 + M_2)}{2M_1}
 * \end{align}
 * \f]
 *
 * Based on the relationship between the reaction Q-value and the conservation laws,
 *
 * \f[
 * \begin{align}
 * \frac{1}{2}M_1 v_{1\mathrm{CM}}^2 + \frac{1}{2}M_2 v_{2\mathrm{CM}}^2 + Q &= \frac{1}{2}M_3 v_{3\mathrm{CM}}^2 + \frac{1}{2}M_4 v_{4\mathrm{CM}}^2 \\
 * M_1 v_{1\mathrm{CM}} &= M_2 v_{2\mathrm{CM}} \\
 * M_3 v_{3\mathrm{CM}} &= M_4 v_{4\mathrm{CM}} \\
 * \end{align}
 * \f]
 *
 * Cancelling \f$ v_{4\mathrm{CM}} \f$, we obtain:
 * \f[
 * \begin{align}
 * \alpha v_{\mathrm{CM}}^2 + Q &= \frac{M_3(M_3 + M_4)}{2M_4}v_{3\mathrm{CM}}^2 \\
 * \alpha v_{\mathrm{CM}}^2 + Q &= \beta v_{3\mathrm{CM}}^2
 * \end{align}
 * \f]
 *
 * Here, \f$ \beta \f$ is defined as:
 *
 * \f[
 * \begin{align}
 * \beta = \frac{M_3(M_3+M_4)}{2M_4}
 * \end{align}
 * \f]
 *
 * For Ion 3, the velocity component perpendicular to the z-axis remains unchanged between the LAB and CM frames,
 * while the parallel component can be transformed using vcm. Therefore,
 *
 * \f[
 * \begin{align}
 * v_{3\mathrm{CM}}\sin{\theta_{\mathrm{CM}}} &= v_3\sin{\theta} \\
 * v_{3\mathrm{CM}}\cos{\theta_{\mathrm{CM}}} &= v_3\cos{\theta} - v_{\mathrm{CM}}
 * \end{align}
 * \f]
 *
 * Cancelling \f$ \theta_{\mathrm{CM}} \f$ from these two equations, we obtain:
 *
 * \f[
 * \begin{align}
 * v_{3\mathrm{cm}}^2 &= v_3^2 -2v_3 v_{\mathrm{CM}}\cos\theta + v_{\mathrm{cm}}^2
 * \end{align}
 * \f]
 *
 * \f$ v_3 \f$ and \f$ \theta \f$ can be considered observable quantities.
 * Cancelling \f$ v_{3\mathrm{CM}} \f$, we obtain:
 *
 * \f[
 * \begin{align}
 * \alpha v_{\mathrm{CM}}^2 + Q = \beta\left(v_3^2 -2v_3v_{\mathrm{cm}}\cos\theta + v_{\mathrm{CM}}^2\right) \\
 * (\alpha - \beta)v_{\mathrm{CM}}^2 + 2(\beta v_3 \cos\theta)v_{\mathrm{CM}} + (Q - \beta v_3^2) = 0
 * \end{align}
 * \f]
 *
 * By solving this quadratic equation, various quantities can be obtained.
 * These relationships are utilized in the methods of this class.
 *
 * In inverse kinematics, ion 4 is detected.
 * Therefore, the indices 3 and 4 should be swapped,
 * and the transformation \f$ \theta_{\mathrm{CM}} \rightarrow \pi - \theta_{\mathrm{CM}} \f$ should be applied.
 */

class TTGTIKProcessor : public TProcessor {
  public:
    /**
     * @brief Constructor.
     */
    TTGTIKProcessor();

    /**
     * @brief Default destructor.
     */
    ~TTGTIKProcessor() override;

    /**
     * @brief Initialize the processor with an event collection.
     * @param col Pointer to the event collection.
     */
    void Init(TEventCollection *col) override;

    /**
     * @brief Main processing function.
     */
    void Process() override;

  private:
    // Collection names
    TString fInputColName;          ///< Name of the input telescope data collection (TTelescopeData)
    TString fInputTrackColName;     ///< Name of the input tracking data collection (TTrack)
    TString fOutputColName;         ///< Name of the output reaction information collection (TReactionInfo)
    TString fDetectorParameterName; ///< Name of the detector parameter (TDetectorParameter)
    TString fTargetParameterName;   ///< Name of the target parameter (TTargetParameter)

    // Data pointers
    TClonesArray *fInData;      ///<! Pointer to the input telescope data (TClonesArray of TTelescopeData)
    TClonesArray *fInTrackData; ///<! Pointer to the input tracking data (TClonesArray of TTrack)
    TClonesArray *fOutData;     ///<! Pointer to the output reaction information (TClonesArray of TReactionInfo)

    // Parameter pointers
    TClonesArray *fDetectorPrm; ///<! Pointer to detector parameter objects (TClonesArray of TDetectorParameter)
    TClonesArray *fTargetPrm;   ///<! Pointer to target parameter objects (TClonesArray of TTargetParameter)

    // Experimental parameters
    Double_t fInitialBeamEnergy; ///< Beam energy immediately after the window (MeV)
    TString fTargetName;         ///< Name of the gas target used in TSrim calculations
    Double_t fPressure;          ///< Gas pressure in Torr
    Double_t fTemperature;       ///< Gas temperature in Kelvin
    IntVec_t fParticleZArray;    ///< Array of atomic numbers for reaction particles
    IntVec_t fParticleAArray;    ///< Array of mass numbers for reaction particles
    Double_t fExcitedEnergy;     ///< Excited state energy (MeV)
    Bool_t fDoCustom;            ///< Flag to enable custom processing
    Bool_t fDoCenterPos;         ///< Flag to use the detector center position

    // TSrim calculator for energy loss computation
    TSrim *srim; ///<! TSrim object to calculate energy loss

    // Constants for bisection method
    const Double_t kInitialMin = -250.0; ///< Initial minimum value for bisection method (mm)
    const Double_t kInitialMax = 1000.0; ///< Initial maximum value for bisection method (mm)
    const Double_t kEpsilon = 1.0e-3;    ///< Convergence threshold for the bisection method
    const Int_t kMaxIteration = 1000;    ///< Maximum number of iterations for the bisection method

    // Mass parameters (set these according to the reaction specifics)
    Double_t M1;
    Double_t M2;
    Double_t M3_default;
    Double_t M3;
    Double_t M4;

    /**
     * @brief Calculate the reaction position along the Z-axis.
     * @param track Pointer to the tracking data (TTrack).
     * @param data Pointer to the telescope data (TTelescopeData).
     * @return Calculated reaction Z position (mm).
     */
    Double_t GetReactionPosition(const TTrack *track, const TTelescopeData *data);

    /**
     * @brief (Deprecated) Newton method for calculating reaction position.
     * @param track Pointer to the tracking data (TTrack).
     * @param data Pointer to the telescope data (TTelescopeData).
     * @return Calculated reaction Z position (mm).
     */
    Double_t newton(const TTrack *, const TTelescopeData *);

    /**
     * @brief Bisection method for calculating reaction position.
     * @param track Pointer to the tracking data (TTrack).
     * @param data Pointer to the telescope data (TTelescopeData).
     * @return Calculated reaction Z position (mm).
     */
    Double_t bisection(const TTrack *track, const TTelescopeData *data);

    /**
     * @brief Target function for the bisection (and Newton) method.
     * Computes the difference between the beam and detected particle center-of-mass energies.
     * @param z Reaction position (mm).
     * @param track Pointer to the tracking data (TTrack).
     * @param data Pointer to the telescope data (TTelescopeData).
     * @return Difference in center-of-mass energy (MeV).
     */
    Double_t TargetFunction(Double_t z, const TTrack *track, const TTelescopeData *data);

    /**
     * @brief Calculate the center-of-mass energy from beam data.
     * @param z Reaction position (mm).
     * @param track Pointer to the tracking data (TTrack).
     * @return Calculated center-of-mass energy (MeV).
     */
    Double_t GetEcmFromBeam(Double_t z, const TTrack *track);

    /**
     * @brief Calculate the LAB energy and LAB angle from detected particle data.
     * @param z Reaction position (mm).
     * @param track Pointer to the tracking data (TTrack).
     * @param data Pointer to the telescope data (TTelescopeData).
     * @return A pair containing the LAB energy (MeV) and LAB angle (radian).
     */
    std::pair<Double_t, Double_t> GetELabALabPair(Double_t z, const TTrack *track, const TTelescopeData *data);

    /**
     * @brief Calculate the center-of-mass energy from detected particle data.
     * @param z Reaction position (mm).
     * @param track Pointer to the tracking data (TTrack).
     * @param data Pointer to the telescope data (TTelescopeData).
     * @return Calculated center-of-mass energy (MeV).
     */
    Double_t GetEcmFromDetectParticle(Double_t z, const TTrack *track, const TTelescopeData *data);

    /**
     * @brief Calculate the center-of-mass energy using relativistic kinematics.
     * @param energy LAB energy (MeV).
     * @param theta LAB angle (radian).
     * @param low_e Lower bound for energy (MeV).
     * @param high_e Upper bound for energy (MeV).
     * @return Calculated center-of-mass energy (MeV).
     */
    Double_t GetEcm_kinematics(Double_t energy, Double_t theta, Double_t low_e, Double_t high_e);

    /**
     * @brief Calculate the center-of-mass energy using classical kinematics.
     * @param energy LAB energy (MeV).
     * @param theta LAB angle (radian).
     * @return Calculated center-of-mass energy (MeV).
     */
    Double_t GetEcm_classic_kinematics(Double_t energy, Double_t theta);

    /**
     * @brief Recalculate the LAB angle after reconstruction.
     * @param Elab LAB energy (MeV).
     * @param Ecm Center-of-mass energy (MeV).
     * @param ALab LAB angle (radian).
     * @return Reconstructed LAB angle (radian).
     */
    Double_t GetCMAngle(Double_t ELab, Double_t Ecm, Double_t ALab);

    /**
     * @brief Generate a custom excited state energy.
     * This function is used for custom processing (e.g., handling excited state effects).
     * @param telID Identifier for the telescope.
     * @param Etotal Total measured energy (MeV).
     * @return Generated excited state energy (MeV).
     */
    Double_t GetCustomExcitedEnergy(Int_t telID, Double_t Etotal);

    // Copy constructor (prohibited)
    TTGTIKProcessor(const TTGTIKProcessor &rhs) = delete;
    // Assignment operator (prohibited)
    TTGTIKProcessor &operator=(const TTGTIKProcessor &rhs) = delete;

    ClassDefOverride(TTGTIKProcessor, 1); ///< ROOT class definition macro.
};
} // namespace art::crib

#endif // end of #ifndef CRIB_TTGTIKPROCESSOR_H_
