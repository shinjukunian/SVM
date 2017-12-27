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
#include "libSVM/svm.h"

// Global Variables




#define Malloc(type,n) (type *)malloc((n)*sizeof(type))
svm_problem makeProblem(waveHndl inputWave, waveHndl inputClasses);



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
    struct svm_parameter params={0};
    params.cache_size=100;
    params.eps=0.001;
    char outPutPath[MAX_PATH_LEN+1]="model.svm";
    int err = 0;
    struct svm_problem problem={0};
    
    // Flag parameters.
    
    if (p->TYPEFlagEncountered && p->svm_type<5) {
        // Parameter: p->svm_type
        params.svm_type=(int)p->svm_type;
    }
    else{
        return INCOMPATIBLE_FLAGS;
        
    }
    
    if (p->KFlagEncountered && p->kernel_type<5) {
        // Parameter: p->kernel_type
        params.kernel_type=(int)p->kernel_type;
    }
    else{
       return INCOMPATIBLE_FLAGS;
    }
    
    if (p->DFlagEncountered) {
        // Parameter: p->degree
        params.degree=(int)p->degree;
    }
    
    if (p->YFlagEncountered) {
        // Parameter: p->gamma
        params.gamma=p->gamma;
    }
    
    if (p->CFFlagEncountered) {
        // Parameter: p->coef0
        params.coef0=p->coef0;
    }
    
    if (p->CFlagEncountered) {
        // Parameter: p->C
        params.C=p->C;
    }
    
    if (p->NUFlagEncountered) {
        // Parameter: p->nu
        params.nu=p->nu;
    }
    
    if (p->SHRINKFlagEncountered) {
        params.shrinking=1;
    }
    
    if (p->PROBFlagEncountered) {
        params.probability=1;
    }
    
    // Main parameters.
    
    if (p->outputPathEncountered) {
        // Parameter: p->outPutPath
        ConcatenatePaths(p->outPutPath, "model.svm", outPutPath);
    }
    else if (XOPSaveFileDialog("Select where to save the model file", "", NULL, "", "svm", outPutPath) != 0){
         return FILE_NOT_FOUND;
    }
   
    
    if (p->inputWaveEncountered && p->inPutWave != NULL) {
        // Parameter: p->inPutWave (test for NULL handle before using)
        if (p->inputClassesEncountered && p->inputClasses != NULL) {
            // Parameter: p->inputClasses (test for NULL handle before using)
            int numDimensionsInputWave;
            int numDimensionsClassesWave;
            CountInt dimensionSizesClassesWave[MAX_DIMENSIONS+1];
            CountInt dimensionSizesInputWave[MAX_DIMENSIONS+1];
            int resultClassesWave=MDGetWaveDimensions(p->inPutWave, &numDimensionsInputWave, dimensionSizesInputWave);
            int resultInPutWave=MDGetWaveDimensions(p->inputClasses, &numDimensionsClassesWave, dimensionSizesClassesWave);
            if (resultInPutWave || resultClassesWave){
                return resultInPutWave;
            }
            else if(numDimensionsInputWave<1){
                return EXPECT_MATRIX;
            }
            else if (dimensionSizesClassesWave[0] != dimensionSizesInputWave[0]){
                return WAVE_LENGTH_MISMATCH;
            }
            else{
                problem=makeProblem(p->inPutWave, p->inputClasses);
                const char *parameterError=svm_check_parameter(&problem,
                                                        &params);
                if (parameterError == 0) {
                    struct svm_model *model=svm_train(&problem, &params);
                    double *target = Malloc(double,problem.l);
                    int total_correct = 0;

                    svm_cross_validation(&problem,&params,10,target);
                    for(int i=0;i<problem.l;i++){
                        if(target[i] == problem.y[i]){
                            ++total_correct;
                        }
                    }
                    free(target);
                    printf("Cross Validation Accuracy = %g%%\n",100.0*total_correct/problem.l);
#ifdef MACIGOR
                    HFSToPosixPath(outPutPath, outPutPath, 0);
#endif
                    if(svm_save_model(outPutPath,model))
                    {
                        fprintf(stderr, "can't save model to file %s\n", outPutPath);
                        
                    }
                    svm_free_and_destroy_model(&model);
                    svm_destroy_param(&params);
                    free(problem.y);
                    free(problem.x);
                }
                else{
                    
                }
                
                
            }
           
           
            
            
        }
    }
    
    
    
    return err;
}


svm_problem makeProblem(waveHndl inputWave, waveHndl inputClasses){
    svm_problem problem={0};
    int numDimensionsInputWave;
    int numDimensionsClassesWave;
    struct svm_node *x_space={0};
    
    CountInt dimensionSizesClassesWave[MAX_DIMENSIONS+1];
    CountInt dimensionSizesInputWave[MAX_DIMENSIONS+1];
    MDGetWaveDimensions(inputWave, &numDimensionsInputWave, dimensionSizesInputWave);
    MDGetWaveDimensions(inputClasses, &numDimensionsClassesWave, dimensionSizesClassesWave);
    int elements=dimensionSizesInputWave[1];
    problem.l=dimensionSizesClassesWave[0];
    problem.y=Malloc(double, problem.l);
    problem.x=Malloc(struct svm_node *,problem.l);
    x_space = Malloc(struct svm_node,elements+1);
    double value[2];
    IndexInt classIndices[MAX_DIMENSIONS]={0};
    IndexInt dataIndices[MAX_DIMENSIONS]={0};
    for (int i=0; i<problem.l; i++) {
        classIndices[0]=i;
        MDGetNumericWavePointValue(inputClasses, classIndices, value);
        problem.y[i]=value[0];
        
        for (int j=0; j<elements; j++) {
            dataIndices[0]=i;
            dataIndices[1]=j;
            MDGetNumericWavePointValue(inputWave, dataIndices, value);
            x_space[j].index=j+1;
            x_space[j].value=value[0];
            
        }
        x_space[elements].index=-1;
        problem.x[i]=&x_space[i];
    }
    
    
    
    return problem;
}



static int
RegisterSVMTrain(void)
{
    const char* cmdTemplate;
    const char* runtimeNumVarList;
    const char* runtimeStrVarList;
    
    // NOTE: If you change this template, you must change the SVMTrainRuntimeParams structure as well.
    cmdTemplate = "SVMTrain /TYPE=number:svm_type /K=number:kernel_type /D=number:degree /Y=number:gamma /CF=number:coef0 /C=number:C /NU=number:nu /SHRINK /PROB outputPath=name:outPutPath, inputWave=wave:inPutWave, inputClasses=wave:inputClasses";
    runtimeNumVarList = "V_flag";
    runtimeStrVarList = "S_fileName";
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
    
    int err;
	
	if (igorVersion < 620) {			// Requires Igor Pro 6.20 or later.
		SetXOPResult(OLD_IGOR);			// OLD_IGOR is defined in SVM.h and there are corresponding error strings in SVM.r and SVMWinCustom.rc.
		return EXIT_FAILURE;
	}
    if (err = RegisterSVMTrain()) {
        SetXOPResult(err);
        return EXIT_FAILURE;
    }

	SetXOPResult(0L);
	return EXIT_SUCCESS;
}
