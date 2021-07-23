#ifndef __ANS_LIB_H__ 
#define __ANS_LIB_H__




typedef void NsHandle;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This function creates an instance to the noise suppression structure
 *
 * Input:
 *      - NS_inst       : Pointer to noise suppression instance that should be
 *                        created
 *
 * Output:
 *      - NS_inst       : Pointer to created noise suppression instance
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int WebRtcNs_Create(NsHandle** NS_inst);


/*
 * This function frees the dynamic memory of a specified noise suppression
 * instance.
 *
 * Input:
 *      - NS_inst       : Pointer to NS instance that should be freed
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int WebRtcNs_Free(NsHandle* NS_inst);


/*
 * This function initializes a NS instance and has to be called before any other
 * processing is made.
 *
 * Input:
 *      - NS_inst       : Instance that should be initialized
 *      - fs            : sampling frequency
 *
 * Output:
 *      - NS_inst       : Initialized instance
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int WebRtcNs_Init(NsHandle* NS_inst, unsigned int fs);

/*
 * This changes the aggressiveness of the noise suppression method.
 *
 * Input:
 *      - NS_inst       : Noise suppression instance.
 *      - mode          : 0: Mild, 1: Medium , 2: Aggressive
 *
 * Output:
 *      - NS_inst       : Updated instance.
 *
 * Return value         :  0 - Ok
 *                        -1 - Error
 */
int WebRtcNs_set_policy(NsHandle* NS_inst, int mode);


/*
 * This functions does Noise Suppression for the inserted speech frame. The
 * input and output signals should always be 10ms (80 or 160 samples).
 *
 * Input
 *      - NS_inst       : Noise suppression instance.
 *      - spframe       : Pointer to speech frame buffer for L band
 *      - spframe_H     : Pointer to speech frame buffer for H band
 *      - fs            : sampling frequency
 *
 * Output:
 *      - NS_inst       : Updated NS instance
 *      - outframe      : Pointer to output frame for L band
 *      - outframe_H    : Pointer to output frame for H band
 *
 * Return value         :  0 - OK
 *                        -1 - Error
 */
int WebRtcNs_Process(NsHandle* NS_inst,
                     short* spframe,
                     short* spframe_H,
                     short* outframe,
                     short* outframe_H);

/* Returns the internally used prior speech probability of the current frame.
 * There is a frequency bin based one as well, with which this should not be
 * confused.
 *
 * Input
 *      - handle        : Noise suppression instance.
 *
 * Return value         : Prior speech probability in interval [0.0, 1.0].
 *                        -1 - NULL pointer or uninitialized instance.
 */
float WebRtcNs_prior_speech_probability(NsHandle* handle);

extern void WebRtcSpl_AnalysisQMF(const int16_t* in_data,
                           int16_t* low_band,
                           int16_t* high_band,
                           int32_t* filter_state1,
                           int32_t* filter_state2);

extern void WebRtcSpl_SynthesisQMF(const int16_t* low_band,
                            const int16_t* high_band,
                            int16_t* out_data,
                            int32_t* filter_state1,
                            int32_t* filter_state2);

#ifdef __cplusplus
}
#endif


#endif
