/* ATIDAQ F/T C Library
 * v1.0.1
 * Copyri
 ght (c) 2001 ATI Industrial Automation
 *
 * The MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/* ftrt.c - routines for realtime calculation of forces and torques from voltages
 */

/*modifications
Dec.23.2004b - Sam Skuce (ATI Industrial Automation) - added #undefs for TRUE and FALSE because they were
causing redefined macro warnings
*/

#define MAX_AXES 6
#define MAX_GAUGES 8

#undef TRUE /*dec.23.2004b - ss*/
#define TRUE 1
#undef FALSE /*dec.23.2004b - ss*/
#define FALSE 0

//-------------------------------------------------
//public interface
typedef struct RTCoefs RTCoefs;
typedef int BOOL;

// calibration information required for F/T conversions
struct RTCoefs {
	unsigned short NumChannels;
	unsigned short NumAxes;
	float working_matrix[MAX_AXES][MAX_GAUGES];
	float bias_slopes[MAX_GAUGES];
	float gain_slopes[MAX_GAUGES];
	float thermistor;
	float bias_vector[MAX_GAUGES+1];
	float TCbias_vector[MAX_GAUGES];
};

void RTConvertToFT(RTCoefs *coefs, float voltages[],float result[],BOOL tempcomp);
void RTBias(RTCoefs *coefs, float voltages[]);

//-------------------------------------------------
//private routines
//void mmult(float *array1, float *array2, float *result,unsigned short r1,unsigned short c1,unsigned short c2);
void mmult(float *a, unsigned short ra, unsigned short ca, unsigned short dca,
		   float *b, unsigned short cb, unsigned short dcb,
		   float *c, unsigned short dcc);
float TempComp(RTCoefs *coefs,float G,float T,unsigned short i);
