#ifndef _ftconfig_h
#define _ftconfig_h

/* ATIDAQ F/T C Library
 * v1.0.2
 * Copyright (c) 2001 ATI Industrial Automation
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

/* ftconfig.h - calibration file and configuration routines
 */

/*
modifications:
Dec.23.2004a - Sam Skuce (ATI Industrial Automation) - explicitly identify cfg member of the Calibration struct
	to be a 'struct Configuration' rather than just use the typedef 'Configuration', because VS.NET 2003 was 
	throwing 'ambigous identifier' errors.
dec.27.2004a - ss - moved the end of the #ifdef __cplusplus block to the end of the file, giving c++ code access to
	some functions that were described as 'private' in the comments 
dec.27.2004b - ss - added namespace daqftclibrary
jan.18.2005a - ss - added support for reading the voltage range of the transducer from the calibration file
aug.5.2005a - ss - added support for reading hardware temperature compensation availability from calibration file
*/

#include "ftrt.h"		// realtime calculations
//#include "dom.h"
#define PI 3.14159265358979

typedef char *Units;
typedef struct Configuration Configuration; 
typedef struct Calibration Calibration;
typedef struct Transform Transform;

// note: tool transforms only supported for 6-axis F/T transducers
struct Transform {
	float TT[6];        // displacement/rotation vector dx, dy, dz, rx, ry, rz
	Units DistUnits;    // units of dx, dy, dz
	Units AngleUnits;   // units of rx, ry, rz
};
// settings that can be changed by the user
struct Configuration {
	Units ForceUnits;        // force units of output
	Units TorqueUnits;       // torque units of output
	Transform UserTransform; // coordinate system transform set by user
	BOOL TempCompEnabled;    // is temperature compensation enabled?
};

// transducer properties read from calibration file
struct Calibration {
	float BasicMatrix[MAX_AXES][MAX_GAUGES];	// non-usable matrix; use rt.working_matrix for calculations
	Units ForceUnits;                           // force units of basic matrix, as read from file; constant
	Units TorqueUnits;                          // torque units of basic matrix, as read from file; constant
	BOOL TempCompAvailable;                     // does this calibration have optional temperature compensation?
	Transform BasicTransform;                   // built-in coordinate transform; for internal use
	float MaxLoads[MAX_AXES];					// maximum loads of each axis, in units above
	char *AxisNames[MAX_AXES];                  // names of each axis
	char *Serial;                               // serial number of transducer (such as "FT4566")
	char *BodyStyle;                            // transducer's body style (such as "Delta")
	char *PartNumber;                           // calibration part number (such as "US-600-3600")
	char *Family;                               // family of transducer (typ. "DAQ")
	char *CalDate;                              // date of calibration
	/*Dec.23.2004a - ss*/
	struct Configuration cfg;                   // struct containing configurable parameters
	RTCoefs rt;                                 // struct containing coefficients used in realtime calculations
	/*Jan.18.2005a - ss*/
	int VoltageRange;							//the voltage range of the transducer
	BOOL BiPolar;								//whether or not this transducer is bipolar
	/*aug.5.2005a - ss*/
	BOOL HWTempComp;							//whether or not this transducer has hardware temperature compensation.
};

#ifdef __cplusplus
/*dec.27.2004b - ss*/
namespace DAQFTCLIBRARY
{
extern "C" {
#endif

	//from dom.h
char *ATI_strdup(const char *s);	// line added by DBL



Calibration *createCalibration(char *CalFilePath, unsigned short index);
// Loads calibration info for a transducer into a new Calibration struct
// Parameters:
//   CalFilePath: the name and path of the calibration file
//   index: the number of the calibration within the file (usually 1)
// Return Values:
//   NULL: Could not load the desired calibration.
// Notes: For each Calibration object initialized by this function, 
//        destroyCalibration must be called for cleanup.

void destroyCalibration(Calibration *cal);
// Frees memory allocated for Calibration struct by a successful
// call to createCalibration.  Must be called when Calibration 
// struct is no longer needed.
// Parameters:
//   cal: initialized Calibration struct

short SetToolTransform(Calibration *cal, float Vector[6],char *DistUnits,char *AngleUnits);
// Performs a 6-axis translation/rotation on the transducer's coordinate system.
// Parameters:
//   cal: initialized Calibration struct
//   Vector: displacements and rotations in the order Dx, Dy, Dz, Rx, Ry, Rz
//   DistUnits: units of Dx, Dy, Dz
//   AngleUnits: units of Rx, Ry, Rz
// Return Values:
//   0: Successful completion
//   1: Invalid Calibration struct
//   2: Invalid distance units
//   3: Invalid angle units

short SetForceUnits(Calibration *cal, char *NewUnits);
// Sets the units of force output
// Parameters:
//   cal: initialized Calibration struct
//   NewUnits: units for force output
//		("lb","klb","N","kN","g","kg")
// Return Values:
//   0: Successful completion
//   1: Invalid Calibration struct
//   2: Invalid force units

short SetTorqueUnits(Calibration *cal, char *NewUnits);
// Sets the units of torque output
// Parameters:
//   cal: initialized Calibration struct
//   NewUnits: units for torque output
//		("in-lb","ft-lb","N-m","N-mm","kg-cm")
// Return Values:
//   0: Successful completion
//   1: Invalid Calibration struct
//   2: Invalid torque units

short SetTempComp(Calibration *cal, int TCEnabled);
// Enables or disables temperature compensation, if available
// Parameters:
//   cal: initialized Calibration struct
//   TCEnabled: 0 = temperature compensation off
//              1 = temperature compensation on
// Return Values:
//   0: Successful completion
//   1: Invalid Calibration struct
//   2: Not available on this transducer system

void Bias(Calibration *cal, float voltages[]);
// Stores a voltage reading to be subtracted from subsequent readings,
// effectively "zeroing" the transducer output to remove tooling weight, etc.
// Parameters:
//   cal: initialized Calibration struct
//   voltages: array of voltages acuired by DAQ system

void ConvertToFT(Calibration *cal, float voltages[],float result[]);
// Converts an array of voltages into forces and torques and
// returns them in result
// Parameters:
//   cal: initialized Calibration struct
//   voltages: array of voltages acuired by DAQ system
//   result: array of force-torque values (typ. 6 elements)



/*dec.27.2004a - ss - These functions aren't 'private' anymore*/

void ResetDefaults(Calibration *cal);
short CalcMatrix(Calibration *cal);
short GetMatrix(Calibration *cal, float *result);
short TTM(Transform xform,float result[6][6],Units ForceUnits,Units TorqueUnits);
float ForceConv(char *Units);
float TorqueConv(char *Units);
float DistConv(char *Units);
float AngleConv(char *Units);
//
//short ReadAttribute(const DOM_Element *elem, char **attValue, char *attName, BOOL required, char *defaultValue);


void Separate(char *ValueList,float results[],unsigned short numValues);
unsigned short FindText(char *str, unsigned short StartPos);
unsigned short FindSpace(char *str, unsigned short StartPos);
char *mid(char *instr,unsigned short startpos,unsigned short length);

/*dec.27.2004a - ss - moved this #ifdef down here, allowing c++ code access to the functions that were
previously marked as 'private' in the comments*/
#ifdef __cplusplus
} /*dec.27.2004b - ss*/
}
#endif

#endif /*#ifndef*/

