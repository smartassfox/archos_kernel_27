/*
    Audio/video-routing-related ivtv functions.
    Copyright (C) 2003-2004  Kevin Thayer <nufan_wfk at yahoo.com>
    Copyright (C) 2005-2007  Hans Verkuil <hverkuil@xs4all.nl>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ivtv-driver.h"
#include "ivtv-i2c.h"
#include "ivtv-cards.h"
#include "ivtv-gpio.h"
#include "ivtv-routing.h"

#include <media/msp3400.h>
#include <media/m52790.h>
#include <media/upd64031a.h>
#include <media/upd64083.h>

/* Selects the audio input and output according to the current
   settings. */
void ivtv_audio_set_io(struct ivtv *itv)
{
	const struct ivtv_card_audio_input *in;
	struct v4l2_routing route;

	/* Determine which input to use */
	if (test_bit(IVTV_F_I_RADIO_USER, &itv->i_flags))
		in = &itv->card->radio_input;
	else
		in = &itv->card->audio_inputs[itv->audio_input];

	/* handle muxer chips */
	route.input = in->muxer_input;
	route.output = 0;
	if (itv->card->hw_muxer & IVTV_HW_M52790)
		route.output = M52790_OUT_STEREO;
	ivtv_i2c_hw(itv, itv->card->hw_muxer, VIDIOC_INT_S_AUDIO_ROUTING, &route);

	route.input = in->audio_input;
	route.output = 0;
	if (itv->card->hw_audio & IVTV_HW_MSP34XX)
		route.output = MSP_OUTPUT(MSP_SC_IN_DSP_SCART1);
	ivtv_i2c_hw(itv, itv->card->hw_audio, VIDIOC_INT_S_AUDIO_ROUTING, &route);
}

/* Selects the video input and output according to the current
   settings. */
void ivtv_video_set_io(struct ivtv *itv)
{
	struct v4l2_routing route;
	int inp = itv->active_input;
	u32 type;

	route.input = itv->card->video_inputs[inp].video_input;
	route.output = 0;
	itv->video_dec_func(itv, VIDIOC_INT_S_VIDEO_ROUTING, &route);

	type = itv->card->video_inputs[inp].video_type;

	if (type == IVTV_CARD_INPUT_VID_TUNER) {
		route.input = 0;  /* Tuner */
	} else if (type < IVTV_CARD_INPUT_COMPOSITE1) {
		route.input = 2;  /* S-Video */
	} else {
		route.input = 1;  /* Composite */
	}

	if (itv->card->hw_video & IVTV_HW_GPIO)
		ivtv_gpio(itv, VIDIOC_INT_S_VIDEO_ROUTING, &route);

	if (itv->card->hw_video & IVTV_HW_UPD64031A) {
		if (type == IVTV_CARD_INPUT_VID_TUNER ||
		    type >= IVTV_CARD_INPUT_COMPOSITE1) {
			/* Composite: GR on, connect to 3DYCS */
			route.input = UPD64031A_GR_ON | UPD64031A_3DYCS_COMPOSITE;
		} else {
			/* S-Video: GR bypassed, turn it off */
			route.input = UPD64031A_GR_OFF | UPD64031A_3DYCS_DISABLE;
		}
		route.input |= itv->card->gr_config;

		ivtv_upd64031a(itv, VIDIOC_INT_S_VIDEO_ROUTING, &route);
	}

	if (itv->card->hw_video & IVTV_HW_UPD6408X) {
		route.input = UPD64083_YCS_MODE;
		if (type > IVTV_CARD_INPUT_VID_TUNER &&
		    type < IVTV_CARD_INPUT_COMPOSITE1) {
			/* S-Video uses YCNR mode and internal Y-ADC, the upd64031a
			   is not used. */
			route.input |= UPD64083_YCNR_MODE;
		}
		else if (itv->card->hw_video & IVTV_HW_UPD64031A) {
		  /* Use upd64031a output for tuner and composite(CX23416GYC only) inputs */
		  if ((type == IVTV_CARD_INPUT_VID_TUNER)||
		      (itv->card->type == IVTV_CARD_CX23416GYC)) {
		    route.input |= UPD64083_EXT_Y_ADC;
		  }
		}
		ivtv_upd64083(itv, VIDIOC_INT_S_VIDEO_ROUTING, &route);
	}
}
