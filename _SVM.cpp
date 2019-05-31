/*	_SVM.c
*/

#include "XOPStandardHeaders.h"			// Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
#include "_SVM.h"
#include "libSVM/svm.h"

#define Malloc(type,n) (type *)malloc((n)*sizeof(type)) //from libSVM

// Helper Function Definitions
svm_problem makeProblem(waveHndl inputWave, waveHndl inputClasses, svm_node *buffer);
void addWeights(waveHndl weights, struct svm_parameter *params);
double classifyNodes(svm_node *nodes, svm_model *model,int predict_probability, double *prob_estimates, int calculateDecisionValues, double* decisionValues);

static void print_string_Igor(const char *s){ // optional output funtion for libSVM to report progress, prints to Igor Pro's Console
    XOPNotice(s);
}




// Operation template: SVMTrain /TYPE=number:svm_type /K=number:kernel_type /D=number:degree /Y=number:gamma /CF=number:coef0 /C=number:C /NU=number:nu /SHRINK /PROB outputPath=name:outPutPath, inputWave=wave:inPutWave, inputClasses=wave:inputClasses

// Runtime param structure for SVMTrain operation.
#pragma pack(2)    // All structures passed to Igor are two-byte aligned.
struct SVMTrainRuntimeParams {
    // Flag parameters.
    
    // Parameters for /TYPE flag group. Types are the same as in svm.h
    int TYPEFlagEncountered;
    double svm_type;
    int TYPEFlagParamsSet[1];
    
    // Parameters for /K flag group. Kernel. Precomputed Kernels are the same as in svm.h. Custom Kernels are not supported so far.
    int KFlagEncountered;
    double kernel_type;
    int KFlagParamsSet[1];
    
    // Parameters for /D flag group. Polynomial Degree in svm_parameter in svm.h
    int DFlagEncountered;
    double degree;
    int DFlagParamsSet[1];
    
    // Parameters for /Y flag group. Gamma in svm_parameter in svm.h
    int YFlagEncountered;
    double gamma;
    int YFlagParamsSet[1];
    
    // Parameters for /CF flag group. Coef0 in svm_parameter in svm.h
    int CFFlagEncountered;
    double coef0;
    int CFFlagParamsSet[1];
    
    // Parameters for /V flag group. Number of cross-validation runs
    int VFlagEncountered;
    double numValidation;
    int VFlagParamsSet[1];
    
    // Parameters for /P flag group. Igor outputPath (url to a folder).
    int PFlagEncountered;
    char outputPath[MAX_OBJ_NAME+1];
    int PFlagParamsSet[1];
    
    // Parameters for /EPSILON flag group. p in svm_parameter in svm.h (for regression)
    int EPSILONFlagEncountered;
    double epsilon;
    int EPSILONFlagParamsSet[1];
    
    // Parameters for /TERM flag group. Epsilon in svm_parameter in svm.h
    int TERMFlagEncountered;
    double eps_term;
    int TERMFlagParamsSet[1];
    
    // Parameters for /C flag group. C  in svm_parameter in svm.h
    int CFlagEncountered;
    double C;
    int CFlagParamsSet[1];
    
    // Parameters for /NU flag group. nu in svm_parameter in svm.h
    int NUFlagEncountered;
    double nu;
    int NUFlagParamsSet[1];
    
    // Parameters for /SHRINK flag group. set shrinking in svm_parameter in svm.h to true or false.
    int SHRINKFlagEncountered;
    // There are no fields for this group because it has no parameters.
    
    // Parameters for /PROB flag group. set probability in svm_parameter in svm.h to true or false.
    int PROBFlagEncountered;
    // There are no fields for this group because it has no parameters.
    
    // Main parameters.
    
    // Parameters for modelName keyword group. Filename of the mdoel outputfile, in combination with /p for the folder URL.
    int modelNameEncountered;
    Handle modelName;
    int modelNameParamsSet[1];
    
    // Parameters for inputWave keyword group. Inputdata, each row is a sapmple, each column contains the sample data points
    int inputWaveEncountered;
    waveHndl inPutWave;
    int inputWaveParamsSet[1];
    
    // Parameters for inputClasses keyword group. Labels for the input data, as integers
    int inputClassesEncountered;
    waveHndl inputClasses;
    int inputClassesParamsSet[1];
    
    // Parameters for weights keyword group. weights for the input data (per row), as float.
    int weightsEncountered;
    waveHndl inputWeights;
    int weightsParamsSet[1];
    
    // These are postamble fields that Igor sets.
    int calledFromFunction;                    // 1 if called from a user function, 0 otherwise.
    int calledFromMacro;                    // 1 if called from a macro, 0 otherwise.
};
typedef struct SVMTrainRuntimeParams SVMTrainRuntimeParams;
typedef struct SVMTrainRuntimeParams* SVMTrainRuntimeParamsPtr;
#pragma pack()    // Reset structure alignment to default.

/*
populates svm_parameter and svm_problem and runs training / validation with the set parameters.
 */

extern "C" int
ExecuteSVMTrain(SVMTrainRuntimeParamsPtr p)
{
    struct svm_parameter params={0};
    params.cache_size=100; //standard values from libSVM (https://github.com/cjlin1/libsvm), 100 MB
    params.eps=0.001; //standard values from libSVM (https://github.com/cjlin1/libsvm)
    char outPutPath[MAX_PATH_LEN+1]="model.svm"; //default file name
    int validationMode=0;
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
    if (p->EPSILONFlagEncountered) {
        // Parameter: p->epsilon
        params.p=p->epsilon;
    }
    
    if (p->TERMFlagEncountered) {
        // Parameter: p->eps_term
        params.eps=p->eps_term;
    }
    else{
        if (params.svm_type == NU_SVC) {
            params.eps=0.00001; //standard values from libSVM (https://github.com/cjlin1/libsvm)
        }
        else{
            params.eps=0.001; //standard values from libSVM (https://github.com/cjlin1/libsvm)
        }
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
    if (p->VFlagEncountered) {
        // Parameter: p->numValidation
        validationMode=(int)p->numValidation;
    }
    
    
    // Main parameters.
    
    if (p->PFlagEncountered && p->modelNameEncountered && p->modelName != NULL) { //build the output path using XOPSupport helper functions (platform independent macOS and Win)
        // Parameter: p->outPutPath
        char fileName[256];
        GetCStringFromHandle(p->modelName, fileName, sizeof(fileName));
        GetFullPathFromSymbolicPathAndFilePath(p->outputPath, fileName, outPutPath);
        
    }
    else if (validationMode<1){
        if(XOPSaveFileDialog("Select where to save the model file", "", NULL, "", "svm", outPutPath) != 0){ // let the user select an output file
            return FILE_NOT_FOUND;
        }
    }
    
   
    
    if (p->inputWaveEncountered && p->inPutWave != NULL) {
        // Parameter: p->inPutWave (test for NULL handle before using)
        if (p->inputClassesEncountered && p->inputClasses != NULL) {
            // Parameter: p->inputClasses (test for NULL handle before using)
            svm_set_print_string_function(&print_string_Igor); //use the Igor Console instead of StdOut
            
            int numDimensionsInputWave;
            int numDimensionsClassesWave;
            
            CountInt dimensionSizesClassesWave[MAX_DIMENSIONS+1];
            CountInt dimensionSizesInputWave[MAX_DIMENSIONS+1];
            
            int resultClassesWave=MDGetWaveDimensions(p->inPutWave, &numDimensionsInputWave, dimensionSizesInputWave);
            int resultInPutWave=MDGetWaveDimensions(p->inputClasses, &numDimensionsClassesWave, dimensionSizesClassesWave);
            
            if (resultInPutWave || resultClassesWave){
                return resultInPutWave; //probably needs a proper error code
            }
            else if(numDimensionsInputWave<1){
                return EXPECT_MATRIX;
            }
            else if (dimensionSizesClassesWave[0] != dimensionSizesInputWave[0]){
                return WAVE_LENGTH_MISMATCH;
            }
            
            else{ // above code checks of the input & label data exists and has the right length and dimensions
                
                int instances=(int)dimensionSizesClassesWave[0]; //number of sample
                int elements=(int)dimensionSizesInputWave[1]; //  number of sample points
                int numPnts=instances*(elements+1); //total number of points + one extra point per sample (for terminator)
                
                struct svm_node *buffer=Malloc(struct svm_node, numPnts); // allocate a buffer svm_node to hold all the sample and label data
                
                problem=makeProblem(p->inPutWave, p->inputClasses, buffer); //populate the buffer in a helper function
                
                if (p->weightsEncountered && p->inputWeights != NULL) { // add weights is specified so
                    addWeights(p->inputWeights, &params);
                }
                
                const char *parameterError=svm_check_parameter(&problem,&params); // use libSVM svm_check_parameter to check for invalid parameters, report output (if any) to user
                
                if (parameterError == NULL) { // no error, proceed training
                    if(validationMode>0){ //validation, don't save model
                        double *target = Malloc(double,problem.l); // a bufer that holds the result from the validation runs,
                        int total_correct = 0;
                        svm_cross_validation(&problem,&params,validationMode,target); // run validation
                        if(params.svm_type == ONE_CLASS){
                            for(int i=0;i<problem.l;i++){ // analyze validation result
                                if(target[i] > 0){ // check of class of validation is in input (known) data, if yes increment correct counter
                                    ++total_correct;
                                }
                            }
                        }
                        else{
                            for(int i=0;i<problem.l;i++){ // analyze validation result
                                if(target[i] == problem.y[i]){ // check of class of validation is equal to input class, if yes increment correct counter
                                    ++total_correct;
                                }
                            }
                        }

                        free(target);
                        
                        double correct=100.0*total_correct/problem.l; //convert to percent, sort of superfluous
                        
                        char notice[1024];
                        snprintf(notice,1024, "Cross Validation Accuracy = %g%%\n",correct);
                        XOPNotice(notice); //igor console output
                        
                        SetOperationNumVar("V_SVMValidation", correct); //igor output of results in a variable
                        
                    }
                    else{
                        struct svm_model *model=svm_train(&problem, &params); // actual training
#ifdef MACIGOR
                        HFSToPosixPath(outPutPath, outPutPath, 0); //convert fileURL to posix (on mac)
#endif
                        if(svm_save_model(outPutPath,model)){ // save model
                            fprintf(stderr, "can't save model to file %s\n", outPutPath);
                        }
                        
                        svm_free_and_destroy_model(&model); //free model memory
                        SetOperationStrVar("S_fileName",outPutPath); //report outputpath to igor
                        svm_set_print_string_function(NULL);
                        
                        char notice[1024];
                        snprintf(notice,1024, "Model saved to %s\n",outPutPath); //report outputpath to igor console
                        XOPNotice(notice);
                    }
  
                }
                else{//cleanup in parameter error case
                    free(problem.y);
                    free(problem.x);
                    free(buffer);
                    svm_destroy_param(&params);
                    XOPNotice(parameterError);
                    return EXPECTED_XOP_PARAM;
                }//cleanup after training, checked for leaks using xcode's instruments
                free(problem.y);
                free(problem.x);
                free(buffer);
                svm_destroy_param(&params);
            }
        }
        else{
            return NULL_WAVE_OP;
        }
    }
    else{
        return NOWAV;
    }

    return err;
}


/*
  helper function to populate svm_parameter with a weights wave. Presumably, the buffer will get deallocated by svm_destroy_param(), if I read the source in svm.cpp correctly.
 */

void addWeights(waveHndl weights, struct svm_parameter *params){
    int numDimsWeightWave;
    CountInt dimSizeWeightWave[MAX_DIMENSIONS+1];
    MDGetWaveDimensions(weights, &numDimsWeightWave, dimSizeWeightWave);
    IndexInt index[MAX_DIMENSIONS]={0};
    if (numDimsWeightWave==2) {
        int weightLabels=(int)dimSizeWeightWave[0];
        params->nr_weight=weightLabels;
        params->weight_label=Malloc(int, weightLabels);
        params->weight=Malloc(double, weightLabels);
        double value[2];
        const char *format="Using weight %f for class %d\n";
        int bufferL=snprintf(NULL, 0, format, 3.402823466e+38F,INT_MAX);
        char *buffer=(char*)malloc(bufferL+1);
        for (int i=0; i<weightLabels; i++) {
            index[0]=i;
            index[1]=0;
            MDGetNumericWavePointValue(weights, index, value);//label
            params->weight_label[i]=(int)value[0];
            index[1]=1;
            MDGetNumericWavePointValue(weights, index, value);//weight
            params->weight[i]=value[0];
            snprintf(buffer, bufferL+1, format,params->weight[i],params->weight_label[i]);
            XOPNotice(buffer);
        }
        free(buffer);
    }
    else{
        const char *error="Weight wave dimensions invalid.\n";
        XOPNotice(error);
    }
   
}

/*
 helper function to populate svm_problem. The buffers are to be managed by ourselves.
 */

svm_problem makeProblem(waveHndl inputWave, waveHndl inputClasses, svm_node *buffer){
    svm_problem problem={0};
    
    int numDimensionsInputWave;
    int numDimensionsClassesWave;
    
    CountInt dimensionSizesClassesWave[MAX_DIMENSIONS+1];
    CountInt dimensionSizesInputWave[MAX_DIMENSIONS+1];
    
    MDGetWaveDimensions(inputWave, &numDimensionsInputWave, dimensionSizesInputWave);
    MDGetWaveDimensions(inputClasses, &numDimensionsClassesWave, dimensionSizesClassesWave);
    
    int elements=(int)dimensionSizesInputWave[1]; // number of data points per sample, number of columns in inputWave
    problem.l=(int)dimensionSizesClassesWave[0]; //number of samples, should be the same as dimensionSizesInputWave[0] number of rows in inputWave or inputClasses
    
    problem.y=Malloc(double, problem.l); //buffer  for the labels
    problem.x=Malloc(struct svm_node *,problem.l); //buffer for the data
    
    double value[2]; // a two-point array, since values can be complex in principle, the real part is at value[0]
    
    IndexInt classIndices[MAX_DIMENSIONS]={0}; //structs to address the data in the input matrices
    IndexInt dataIndices[MAX_DIMENSIONS]={0};
    int counter=0;
    
    for (int i=0; i<problem.l; i++) {
        
        problem.x[i]=&buffer[counter]; // assign the address of the current node to the problem
    
        classIndices[0]=i; // we want the point at row (sample) i
        
        MDGetNumericWavePointValue(inputClasses, classIndices, value); // get label at i
        
        problem.y[i]=value[0]; // label node at i
        
        for (int j=0; j<elements; j++) {// get the sample data by iterating over all data points
            dataIndices[0]=i;// sample number
            dataIndices[1]=j;// data point number
            MDGetNumericWavePointValue(inputWave, dataIndices, value);//get data point
            buffer[counter].index=j+1; // data point index, of current node,one based
            buffer[counter].value=value[0]; //data point valu of current node
            counter++;
            
        }
        buffer[counter++].index=-1; // after writing all the data, add a terminator node with index -1
    }
    
    return problem;
}




// Operation template: SVMClassify /PROB /DEC /P=name:pathName modelName=string:modelname, inputWave=wave:inPutWave
// Structure to hold the parameters for svm classification

// Runtime param structure for SVMClassify operation.
#pragma pack(2)    // All structures passed to Igor are two-byte aligned.
struct SVMClassifyRuntimeParams {
    // Flag parameters.
    
    // Parameters for /PROB flag group. report probability values, if the trained model permits.
    int PROBFlagEncountered;
    // There are no fields for this group because it has no parameters.
    
    // Parameters for /DEC flag group. report decision values
    int DECFlagEncountered;
    // There are no fields for this group because it has no parameters.
    
    // Parameters for /P flag group. // url for the folder holding the model
    int PFlagEncountered;
    char pathName[MAX_OBJ_NAME+1];
    int PFlagParamsSet[1];
    // Main parameters.
    
    // Parameters for modelName keyword group. filename of model
    int modelNameEncountered;
    Handle modelname;
    int modelNameParamsSet[1];
    
    // Parameters for inputWave keyword group. inputdata to classify, same format as for training
    int inputWaveEncountered;
    waveHndl inPutWave;
    int inputWaveParamsSet[1];
    
    // These are postamble fields that Igor sets.
    int calledFromFunction;                    // 1 if called from a user function, 0 otherwise.
    int calledFromMacro;                    // 1 if called from a macro, 0 otherwise.
};
typedef struct SVMClassifyRuntimeParams SVMClassifyRuntimeParams;
typedef struct SVMClassifyRuntimeParams* SVMClassifyRuntimeParamsPtr;
#pragma pack()    // Reset structure alignment to default.

/*
 ExecuteSVMClassify load the model and runs classification / regression of the input data.
 */

extern "C" int
ExecuteSVMClassify(SVMClassifyRuntimeParamsPtr p)
{
    int err = 0;
    char inPutPath[MAX_PATH_LEN+1]="";
    struct svm_model *model=NULL;
    struct svm_node *nodes=NULL;
    int predict_probability=0;
    int calculateDecisionValues=0;
    
    if (p->PROBFlagEncountered) { // we want probability values in the output
        predict_probability=1;
    }
    
    if(p->DECFlagEncountered){
        calculateDecisionValues=1;
    }
    
    
    //locating the model an building the model fileURL
    if (p->PFlagEncountered && p->modelNameEncountered && p->modelname != NULL) {
        // Parameter: p->modelPath
        char fileName[256];
        GetCStringFromHandle(p->modelname, fileName, sizeof(fileName));
        GetFullPathFromSymbolicPathAndFilePath(p->pathName, fileName, inPutPath);
    }
    else if(XOPOpenFileDialog("Select the model file", "", NULL, "", inPutPath) != 0){//prompt user
        return FILE_NOT_FOUND;
    }
#ifdef MACIGOR
    HFSToPosixPath(inPutPath, inPutPath, 0); //platform specific URL conversion
#endif
    
    model=svm_load_model(inPutPath); // actually load the model
    
    if (model == NULL) {
        return FILE_OPEN_ERROR; // if we failed to load the model, abort
    }

    if (p->inputWaveEncountered) {
        if (p->inPutWave != NULL) {//check if our input data is not NULL
            
            svm_set_print_string_function(&print_string_Igor); // print from libSVM to the igor console
            
            int numDimensionsInputWave;
            CountInt dimensionSizesInputWave[MAX_DIMENSIONS+1];
            MDGetWaveDimensions(p->inPutWave, &numDimensionsInputWave, dimensionSizesInputWave); // get size of input data
            
            waveHndl probWave=NULL;
            waveHndl decWave=NULL;
            int elements; //samples
            int points; //points per sample
            
            int numClasses=svm_get_nr_class(model); // classes in model (needed for allocating probWave)
            int numberOfDecisionValues=(numClasses*(numClasses-1))/2;
            
            double *prob_estimates=(double *) malloc(numClasses*sizeof(double)); //buffer to hold prob estimates
            double *decisionValues=(double *) malloc(numberOfDecisionValues*sizeof(double)); //buffer to hold prob estimates
            
            if (numDimensionsInputWave>1) { //classify a matrux of sample vectors
                
                elements=(int)dimensionSizesInputWave[0];
                points=(int)dimensionSizesInputWave[1];
                
                waveHndl outWave; // hold the classification result
                
                if (predict_probability) { // we want probability data, allocate the requires structures
                    if(svm_check_probability_model(model)){// the model supports probability data
                        CountInt probSize[MAX_DIMENSIONS+1]={0};
                        probSize[0]=elements;//probability output matrix. same number of rows as our input data
                        probSize[1]=numClasses;//probability output matrix. one columns per class
                        MDMakeWave(&probWave, "M_SVMProb", NULL, probSize, NT_FP32, 1);// make a wave (igor pro buffer) with the correct dimensions
                        
                        //properly label each column with the sample class
                        int *labels=Malloc(int, numClasses);
                        svm_get_labels(model, labels);
                        int bLength=snprintf(NULL, 0, "%d",INT_MAX);
                        char *buffer=(char*)malloc(bLength+1);
                        for (int i=0; i<numClasses; i++) {
                            snprintf(buffer,bLength+1, "%d",labels[i]);
                            MDSetDimensionLabel(probWave, 1, i, buffer);
                        }
                        free(buffer);
                        free(labels);
                    }
                    else{// the model doesnt support prob data, report error
                        char buffer[1024];
                        snprintf(buffer,1024, "The selected model does not contain any probability data. Only classification results will be returned");
                        XOPNotice(buffer);
                    }
                }
                
                if (calculateDecisionValues) {
                    CountInt decSize[MAX_DIMENSIONS+1]={0};
                    decSize[0]=elements;
                    decSize[1]=numberOfDecisionValues;
                    MDMakeWave(&decWave, "M_SVMDec", NULL, decSize, NT_FP32, 1);// make a wave (igor pro buffer) with the correct dimensions
                    
                    int *labels=Malloc(int, numClasses);
                    svm_get_labels(model, labels);
                    int bLength=snprintf(NULL, 0, "Dec %d-%d",INT_MAX,INT_MAX);
                    char *buffer=(char*)malloc(bLength+1);
                    
                    int p=0;
                    for (int i=0; i<numClasses; i++) {
                        for (int j=i+1; j<numClasses; j++) {
                            snprintf(buffer,bLength+1, "Dec %d-%d",labels[i],labels[j]);
                            MDSetDimensionLabel(decWave, 1, p, buffer);
                            p++;
                        }
                    }
                    free(buffer);
                    free(labels);
                    
                }
                
                MakeWave(&outWave, "W_SVMResult", elements, NT_FP32, 1); // data structure to hold the classification result
                
                double result[2]={0}; // holds the classification result. to hand it over to igor, it has to be a 2 point array, since data could be complex
                
                nodes=Malloc(struct svm_node,points+1); // a buffer to hold the data to classify
                //structs to locate the data in the data to classify
                IndexInt dataIndices[MAX_DIMENSIONS]={0};
                IndexInt outIndices[MAX_DIMENSIONS]={0};
                
                
                double value[2]={0};
                for (int j=0; j<elements; j++) {
                    for (int i=0; i<points; i++) {
                        dataIndices[1]=i;
                        dataIndices[0]=j;
                        MDGetNumericWavePointValue(p->inPutWave, dataIndices, value);
                        nodes[i].index=i+1;
                        nodes[i].value=value[0];
                    }
                    nodes[points].index=-1; //populate the input buffer with one sample (see makeProblem())
                    
                    result[0]=classifyNodes(nodes, model, predict_probability, prob_estimates,calculateDecisionValues,decisionValues); //classify sample with probability estimates
                    outIndices[0]=j;// set igor output index (row number)
                    MDSetNumericWavePointValue(outWave, outIndices, result); //write data back to igor
                    
                    if (predict_probability && probWave != NULL) {
                        IndexInt probIndices[MAX_DIMENSIONS]={0};
                        double v[2]={0};// holds the probability estimate
                        for (int n=0; n<numClasses; n++) { // iterate over the probability estimate of each class and write it into the igor matrix
                            probIndices[0]=j;
                            probIndices[1]=n;
                            v[0]=prob_estimates[n];
                            MDSetNumericWavePointValue(probWave, probIndices, v);
                        }
                    }
                    
                    if (calculateDecisionValues == 1 && decWave != NULL) {
                        IndexInt decIndices[MAX_DIMENSIONS]={0};
                        double v[2]={0};// holds the decision value estimate
                        for (int n=0; n<numberOfDecisionValues; n++) {
                            decIndices[0]=j;
                            decIndices[1]=n;
                            v[0]=decisionValues[n];
                            MDSetNumericWavePointValue(decWave, decIndices, v);
                        }
                    }
                }
                free(nodes);
                
                
            }
            else{// classify only one sample vector, report in a variable in igor
                points=(int)dimensionSizesInputWave[0];
                nodes=Malloc(struct svm_node,points+1);
                IndexInt dataIndices[MAX_DIMENSIONS]={0};
                double value[2]={0};
                for (int i=0; i<points; i++) {
                    dataIndices[0]=i;
                    MDGetNumericWavePointValue(p->inPutWave, dataIndices, value);
                    nodes[i].index=i+1;
                    nodes[i].value=value[0];
                }
                nodes[points].index=-1;
                double *prob_estimates=NULL;
                double result=classifyNodes(nodes, model, predict_probability, prob_estimates,calculateDecisionValues,decisionValues);
                SetOperationNumVar("V_SVMClass",result);
                free(nodes);
            }
            svm_set_print_string_function(NULL);
            svm_free_and_destroy_model(&model);
            free(prob_estimates);
            free(decisionValues);
        }
        else{
            return NULL_WAVE_OP;
        }
        // Parameter: p->inPutWave (test for NULL handle before using)
        
    }
    else{
        return NOWAV;
    }
    
   
    
    return err;
}

/*
 helper function to run the classification of sample vector *nodes with *model. optionally report probability estimates into *prob_estimates (needs to be allocated and appropriately sized)
 */

double classifyNodes(svm_node *nodes, svm_model *model,int predict_probability, double *prob_estimates, int calculateDecisionValues, double* decisionValues){
    
    int svm_type=svm_get_svm_type(model);
    double predict_label;
    //int nr_class=svm_get_nr_class(model);
    //int j=0;
    
    if (predict_probability && (svm_type==C_SVC || svm_type==NU_SVC)){ // only these types of models support prob estimates in the first place
        predict_label = svm_predict_probability(model,nodes,prob_estimates); // predict with probability estimates
    }
    
    if (calculateDecisionValues && decisionValues != NULL) {
        predict_label = svm_predict_values(model, nodes, decisionValues);
    }
    else{
        predict_label = svm_predict(model,nodes); // predict without estimates
    }
    
    return predict_label;
}


/*
 Igor pro specific functions
 */

static int
RegisterSVMClassify(void)
{
    const char* cmdTemplate;
    const char* runtimeNumVarList;
    const char* runtimeStrVarList;
    
    // NOTE: If you change this template, you must change the SVMClassifyRuntimeParams structure as well.
    cmdTemplate = "SVMClassify /PROB /DEC /P=name:pathName modelName=string:modelname, inputWave=wave:inPutWave";
    runtimeNumVarList = "V_SVMClass;V_SVMProb";
    runtimeStrVarList = "";
    return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SVMClassifyRuntimeParams), (void*)ExecuteSVMClassify, 0);
}

static int
RegisterSVMTrain(void)
{
    const char* cmdTemplate;
    const char* runtimeNumVarList;
    const char* runtimeStrVarList;
    
    // NOTE: If you change this template, you must change the SVMTrainRuntimeParams structure as well.
    cmdTemplate = "SVMTrain /TYPE=number:svm_type /K=number:kernel_type /D=number:degree /Y=number:gamma /CF=number:coef0 /V=number:numValidation /P=name:outputPath /EPSILON=number:epsilon /TERM=number:eps_term /C=number:C /NU=number:nu /SHRINK /PROB modelName=String:modelName, inputWave=wave:inPutWave, inputClasses=wave:inputClasses, weights=wave:inputWeights";
    runtimeNumVarList = "V_SVMValidation;V_SVMNumSupportVectors";
    runtimeStrVarList = "S_fileName";
    return RegisterOperation(cmdTemplate, runtimeNumVarList, runtimeStrVarList, sizeof(SVMTrainRuntimeParams), (void*)ExecuteSVMTrain, 0);
}


static XOPIORecResult
RegisterFunction()
{
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
	return 0;
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
    if (err = RegisterSVMClassify()) {
        SetXOPResult(err);
        return EXIT_FAILURE;
    }
    

	SetXOPResult(0L);
	return EXIT_SUCCESS;
}
