#ifndef OBS_FILTER_REVERB_H
#define OBS_FILTER_REVERB_H

#ifndef __cplusplus
struct Reverb;
typedef struct Reverb Reverb;
#else
class Reverb;
#endif

struct ReverbParameters
{
	float roomSize;     /**< Room size, 0 to 1.0, where 1.0 is big, 0 is small. */
	float damping;      /**< Damping, 0 to 1.0, where 0 is not damped, 1.0 is fully damped. */
	float wetLevel;     /**< Wet level, 0 to 1.0 */
	float dryLevel;     /**< Dry level, 0 to 1.0 */
	float width;        /**< Reverb width, 0 to 1.0, where 1.0 is very wide. */
	float freezeMode;   /**< Freeze mode - values < 0.5 are "normal" mode, values > 0.5
						put the reverb into a continuous feedback loop. */
};

#ifdef __cplusplus
extern "C"
{
#endif
Reverb* bili_reverb_create();
void bili_reverb_destroy(Reverb* instance);
void bili_reverb_set_sample_rate(Reverb* instance, double val);
void bili_reverb_reset(Reverb* instance);
void bili_reverb_process(Reverb* instance, float *left, float *right, int numSample);
void bili_reverb_set_parameters(Reverb* instance, struct ReverbParameters *val);
#ifdef __cplusplus
}
#endif

#endif