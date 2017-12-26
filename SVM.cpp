/*	SVM.c

	Igor Pro 3.0 extended wave data storage from 1 dimension to 4 dimensions.
	This sample XOP illustrates how to access waves of dimension 1 through 4.
	
	The function WAGetWaveInfo illustrates the use of the following calls:
		WaveName, WaveType, WaveUnits, WaveScaling
		MDGetWaveDimensions, MDGetWaveScaling, MDGetWaveUnits, MDGetDimensionLabels
		
	It will  with Igor Pro 2.0 or later.

	Invoke it from Igor's command line like this:
		Make/N=(5,4,3) wave3D		// wave with 5 rows, 4 columns and 3 layers
		Print WAGetWaveInfo(wave3D)
	
	The functions WAFill3DWaveDirectMethod, WAFill3DWavePointMethod and
	WAFill3DWaveStorageMethod each fill a 3D wave with values, using different
	wave access methods. They all require Igor Pro 3.0 or later.
	
	You can invoke these functions from Igor Pro 3.0 or later as follows:
		Edit wave3D
		WAFill3DWaveDirectMethod(wave3D)
	or	WAFill3DWavePointMethod(wave3D)
	or	WAFill3DWaveStorageMethod(wave3D)
	
	The function fills a 3 dimensional wave with values such that:
		w[p][q][r] = p + 1e3*q + 1e6*r
	where p is the row number, q is the column number and r is the layer number.
	This is the equivalent of executing the following in Igor Pro 3.0 or later:
		wave3D = p + 1e3*q + 1e6*r
	
	The function WAModifyTextWave shows how to read and write the contents of
	a text wave. Invoke it from Igor Pro 3.0 or later like this:
		Make/T/N=(4,4) textWave2D = "(" + num2str(p) + "," + num2str(q) + ")"
		Edit textWave2D
		WAModifyTextWave(textWave2D, "Row/col=", ".")
	
	HR, 091021
		Updated for 64-bit compatibility.

	HR, 2013-02-08
		Updated for Xcode 4 compatibility. Changed to use XOPMain instead of main.
		As a result the XOP now requires Igor Pro 6.20 or later.
*/

#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "SVM.h"

// Global Variables





/*	RegisterFunction()
	
	Igor calls this at startup time to find the address of the
	XFUNCs added by this XOP. See XOP manual regarding "Direct XFUNCs".
*/



// Operation template: SVMTrain /TYPE=number:svm_type /K=number:kernel_type /D=number:degree /Y=number:gamma /CF=number:coef0 /C=number:C /NU=number:nu /SHRINK /PROB outputPath=name:outPutPath, inputWave=wave:inPutWave, inputClasses=wave:inputClasses

// Runtime param structure for SVMTrain operation.
#pragma pack(2)    // All structures passed to Igor are two-byte aligned.
struct SVMTrainRuntimeParams {
    // Flag parameters.
    
    // Parameters for /TYPE flag group.
    int TYPEFlagEncountered;
    double svm_type;
    int TYPEFlagParamsSet[1];
    
    // Parameters for /K flag group.
    int KFlagEncountered;
    double kernel_type;
    int KFlagParamsSet[1];
    
    // Parameters for /D flag group.
    int DFlagEncountered;
    double degree;
    int DFlagParamsSet[1];
    
    // Parameters for /Y flag group.
    int YFlagEncountered;
    double gamma;
    int YFlagParamsSet[1];
    
    // Parameters for /CF flag group.
    int CFFlagEncountered;
    double coef0;
    int CFFlagParamsSet[1];
    
    // Parameters for /C flag group.
    int CFlagEncountered;
    double C;
    int CFlagParamsSet[1];
    
    // Parameters for /NU flag group.
    int NUFlagEncountered;
    double nu;
    int NUFlagParamsSet[1];
    
    // Parameters for /SHRINK flag group.
    int SHRINKFlagEncountered;
    // There are no fields for this group because it has no parameters.
    
    // Parameters for /PROB flag group.
    int PROBFlagEncountered;
    // There are no fields for this group because it has no parameters.
    
    // Main parameters.
    
    // Parameters for outputPath keyword group.
    int outputPathEncountered;
    char outPutPath[MAX_OBJ_NAME+1];
    int outputPathParamsSet[1];
    
    // Parameters for inputWave keyword group.
    int inputWaveEncountered;
    waveHndl inPutWave;
    int inputWaveParamsSet[1];
    
    // Parameters for inputClasses keyword group.
    int inputClassesEncountered;
    waveHndl inputClasses;
    int inputClassesParamsSet[1];
    
    // These are postamble fields that Igor sets.
    int calledFromFunction;                    // 1 if called from a user function, 0 otherwise.
    int calledFromMacro;                    // 1 if called from a macro, 0 otherwise.
};
typedef struct SVMTrainRuntimeParams SVMTrainRuntimeParams;
typedef struct SVMTrainRuntimeParams* SVMTrainRuntimeParamsPtr;
#pragma pack()    // Reset structure alignment to default.

extern "C" int
ExecuteSVMTrain(SVMTrainRuntimeParamsPtr p)
{
    int err = 0;
    
    // Flag parameters.
    
    if (p->TYPEFlagEncountered) {
        // Parameter: p->svm_type
    }
    
    if (p->KFlagEncountered) {
        // Parameter: p->kernel_type
    }
    
    if (p->DFlagEncountered) {
        // Parameter: p->degree
    }
    
    if (p->YFlagEncountered) {
        // Parameter: p->gamma
    }
    
    if (p->CFFlagEncountered) {
        // Parameter: p->coef0
    }
    
    if (p->CFlagEncountered) {
        // Parameter: p->C
    }
    
    if (p->NUFlagEncountered) {
        // Parameter: p->nu
    }
    
    if (p->SHRINKFlagEncountered) {
    }
    
    if (p->PROBFlagEncountered) {
    }
    
    // Main parameters.
    
    if (p->outputPathEncountered) {
        // Parameter: p->outPutPath
    }
    
    if (p->inputWaveEncountered) {
        // Parameter: p->inPutWave (test for NULL handle before using)
    }
    
    if (p->inputClassesEncountered) {
        // Parameter: p->inputClasses (test for NULL handle before using)
    }
    
    return err;
}

static int
RegisterSVMTrain(void)
{
    const char* cmdTemplate;
    const char* runtimeNumVarList;
    const char* runtimeStrVarList;
    
    // NOTE: If you change this template, you must change the SVMTrainRuntimeParams structure as well.
    cmdTemplate = "SVMTrain /TYPE=number:svm_type /K=number:kernel_type /D=number:degree /Y=number:gamma /CF=number:coef0 /C=number:C /NU=number:nu /SHRINK /PROB outputPath=name:outPutPath, inputWave=wave:inPutWave, inputClasses=wave:inputClasses";
    runtimeNumVarList = "";
    runtimeStrVarList = "";
    return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SVMTrainRuntimeParams), (void*)ExecuteSVMTrain, 0);
}





static XOPIORecResult
RegisterFunction()
{
	int funcIndex;

	funcIndex = (int)GetXOPItem(0);		// Which function is Igor asking about?
//    switch (funcIndex) {
//        case 0:                        // WAGetWaveInfo(wave)
//            return (XOPIORecResult)WAGetWaveInfo;
//            break;
//        case 1:                        // WAFill3DWaveDirectMethod(wave)
//            return (XOPIORecResult)WAFill3DWaveDirectMethod;
//            break;
//        case 2:                        // WAFill3DWavePointMethod(wave)
//            return (XOPIORecResult)WAFill3DWavePointMethod;
//            break;
//        case 3:                        // WAFill3DWaveStorageMethod(wave)
//            return (XOPIORecResult)WAFill3DWaveStorageMethod;
//            break;
//        case 4:                        // WAModifyTextWave(wave, prependString, appendString)
//            return (XOPIORecResult)WAModifyTextWave;
//            break;
//    }
	return 0;
}

/*	DoFunction()
	
	Igor calls this when the user invokes one if the XOP's XFUNCs
	if we returned NIL for the XFUNC from RegisterFunction. In this
	XOP, we always use the direct XFUNC method, so Igor will never call
	this function. See XOP manual regarding "Direct XFUNCs".
*/
static int
DoFunction()
{
	int funcIndex;
	void *p;				// Pointer to structure containing function parameters and result.
	int err;

	funcIndex = (int)GetXOPItem(0);	// Which function is being invoked ?
	p = (void*)GetXOPItem(1);		// Get pointer to params/result.
//    switch (funcIndex) {
//        case 0:                        // WAGetWaveInfo(wave)
//            err = WAGetWaveInfo((WAGetWaveInfoParams*)p);
//            break;
//        case 1:                        // WAFill3DWaveDirectMethod(wave)
//            err = WAFill3DWaveDirectMethod((WAFill3DWaveDirectMethodParams*)p);
//            break;
//        case 2:                        // WAFill3DWavePointMethod(wave)
//            err = WAFill3DWavePointMethod((WAFill3DWavePointMethodParams*)p);
//            break;
//        case 3:                        // WAFill3DWaveStorageMethod(wave)
//            err = WAFill3DWaveStorageMethod((WAFill3DWaveStorageMethodParams*)p);
//            break;
//        case 4:                        // WAModifyTextWave(wave, prependString, appendString)
//            err = WAModifyTextWave((WAModifyTextWaveParams*)p);
//            break;
//    }
	return(err);
}

/*	XOPEntry()

	This is the entry point from the host application to the XOP for all messages after the
	INIT message.
*/
extern "C" void
XOPEntry(void)
{	
	XOPIORecResult result = 0;

	switch (GetXOPMessage()) {
		case FUNCTION:						// Our external function being invoked ?
			result = DoFunction();
			break;

		case FUNCADDRS:
			result = RegisterFunction();
			break;
	}
	SetXOPResult(result);
}

/*	XOPMain(ioRecHandle)

	This is the initial entry point at which the host application calls XOP.
	The message sent by the host must be INIT.
	
	XOPMain does any necessary initialization and then sets the XOPEntry field of the
	ioRecHandle to the address to be called for future messages.
*/
HOST_IMPORT int
XOPMain(IORecHandle ioRecHandle)		// The use of XOPMain rather than main means this XOP requires Igor Pro 6.20 or later
{	
	XOPInit(ioRecHandle);				// Do standard XOP initialization.
	SetXOPEntry(XOPEntry);				// Set entry point for future calls.
	
	if (igorVersion < 620) {			// Requires Igor Pro 6.20 or later.
		SetXOPResult(OLD_IGOR);			// OLD_IGOR is defined in SVM.h and there are corresponding error strings in SVM.r and SVMWinCustom.rc.
		return EXIT_FAILURE;
	}

	SetXOPResult(0L);
	return EXIT_SUCCESS;
}
