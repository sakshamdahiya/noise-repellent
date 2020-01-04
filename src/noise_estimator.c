/*
noise-repellent -- Noise Reduction LV2

Copyright 2016 Luciano Dato <lucianodato@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/
*/

/**
* \file estimate_noise_spectrum.c
* \author Luciano Dato
* \brief Abstraction noise spectrum estimation
*/

#include <math.h>
#include <float.h>

#include "extra_functions.c"

/**
* Noise estimator struct.
*/
typedef struct
{
	//General parameters
	int fft_size;
	int half_fft_size;

	//noise related
	float *noise_spectrum;		   //captured noise profile power spectrum
	bool noise_spectrum_available; //indicate whether a noise profile is available or no
	float noise_block_count;	   //Count windows for mean computing
} NoiseEstimator;

void n_e_reset(NoiseEstimator *self)
{
	self->noise_spectrum_available = false;
	initialize_array(self->noise_spectrum, 0.f, self->half_fft_size + 1);
}

NoiseEstimator *n_e_init(int fft_size)
{
	NoiseEstimator *self = (NoiseEstimator *)malloc(sizeof(NoiseEstimator));

	//Configuration
	self->fft_size = fft_size;
	self->half_fft_size = self->fft_size / 2;

	self->noise_spectrum = (float *)calloc((self->half_fft_size + 1), sizeof(float));

	n_e_reset(self);

	return self;
}

void n_e_free(NoiseEstimator *self)
{
	free(self->noise_spectrum);
	free(self);
}

bool n_e_available(NoiseEstimator *self)
{
	return self->noise_spectrum_available;
}

/**
* Noise estimation using a rolling mean over user selected noise section.
*/
void n_e_run(NoiseEstimator *self, float *spectrum)
{
	int k;

	//Increase window count for rolling mean
	self->noise_block_count++;

	//Get noise thresholds based on averageing the input noise signal between frames
	for (k = 0; k <= self->half_fft_size; k++)
	{
		if (self->noise_block_count <= 1.f)
		{
			self->noise_spectrum[k] = spectrum[k];
		}
		else
		{
			self->noise_spectrum[k] += ((spectrum[k] - self->noise_spectrum[k]) / self->noise_block_count);
		}
	}

	//Now we have an estimated noise spectrum
	self->noise_spectrum_available = true;
}