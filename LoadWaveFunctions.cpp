//
//  LoadWaveFunctions.c
//  SVM
//
//  Created by Morten Bertz on 2017/12/26.
//

#include "LoadWaveFunctions.h"
#include "XOPStandardHeaders.h"            // Include ANSI headers, Mac headers, IgorXOP.h, XOP.h and XOPSupport.h
static int gCallSpinProcess = 1;        // Set to 1 to all user abort (cmd dot) and background processing.

static int
AddCStringToHandle(                        // Concatenates C string to handle.
    const char *theStr,
    Handle theHand)
{
    return PtrAndHand(theStr, theHand, strlen(theStr));
}

#pragma pack(2)        // All structures passed to Igor are two-byte aligned
struct WAGetWaveInfoParams {
    waveHndl w;
    Handle strH;
};
typedef struct WAGetWaveInfoParams WAGetWaveInfoParams;
#pragma pack()        // Reset structure alignment to default.

extern "C" int
WAGetWaveInfo(WAGetWaveInfoParams* p)    // See the top of the file for instructions on how to invoke this function from Igor Pro 3.0 or later.
{
    char buf[256];
    char waveName[MAX_OBJ_NAME+1];
    int waveType;
    int dimension, numDimensions;
    CountInt dimensionSizes[MAX_DIMENSIONS+1];
    char dimensionUnits[MAX_DIMENSIONS][MAX_UNIT_CHARS+1];
    IndexInt element;
    char dataUnits[MAX_UNIT_CHARS+1];
    double dataFullScaleMax, dataFullScaleMin;
    double sfA[MAX_DIMENSIONS];
    double sfB[MAX_DIMENSIONS];
    char dimLabel[MAX_DIM_LABEL_CHARS+1];
    int result;

    if (p->w==NIL) {
        p->strH = NIL;                        // Tell Igor that function return value is undefined.
        return NON_EXISTENT_WAVE;            // Make sure wave exists.
    }

    p->strH = NewHandle(0L);
    if (p->strH == NIL)
        return NOMEM;

    // Get wave name.
    WaveName(p->w, waveName);

    // Get wave data type.
    waveType = WaveType(p->w);

    // Get number of used dimensions in wave.
    if (result = MDGetWaveDimensions(p->w, &numDimensions, dimensionSizes))
        return result;

    /*    Get wave scaling for all used dimensions.
        The scaled index value for point p of dimension d is computed as:
            scaled index = p*sfA[d] + sfB[d];
    */
    for(dimension=0; dimension<numDimensions; dimension++) {
        if (result = MDGetWaveScaling(p->w, dimension, &sfA[dimension], &sfB[dimension]))
            return result;
    }

    // Get units for all dimensions.
    for(dimension=0; dimension<numDimensions; dimension++) {
        if (result = MDGetWaveUnits(p->w, dimension, &dimensionUnits[dimension][0]))
            return result;
    }

    /*    Get the data nominal full scale values for the wave.
        -1 means get full scale values instead of dimension scaling.
    */
    if (result = MDGetWaveScaling(p->w, -1, &dataFullScaleMax, &dataFullScaleMin))
        return result;

    /*    Get the data units for the wave.
        -1 means data units instead of dimension units.
    */
    if (result = MDGetWaveUnits(p->w, -1, dataUnits))
        return result;

    // Now, store all of the info in the handle to return to Igor.

    sprintf(buf, "Wave name: \'%s\'; type: %d; dimensions: %d", waveName, waveType, numDimensions);
    if (result = AddCStringToHandle(buf, p->strH))
        return result;

    // Add the data units and nominal full scale values.
    sprintf(buf, "; data units=\"%s\"; data full scale=%g,%g", dataUnits, dataFullScaleMin, dataFullScaleMax);
    if (result = AddCStringToHandle(buf, p->strH))
        return result;

    // Add information for each dimension.
    for(dimension=0; dimension<numDimensions; dimension++) {
        if (result = AddCStringToHandle(CR_STR, p->strH))            // Add CR.
            return result;
        sprintf(buf, "\tDimension number: %d, size=%lld, sfA=%g, sfB=%g, dimensionUnits=\"%s\"" CR_STR,
                dimension, (SInt64)dimensionSizes[dimension], sfA[dimension], sfB[dimension], dimensionUnits[dimension]);
        if (result = AddCStringToHandle(buf, p->strH))
            return result;

        //    Get dimension label for each element of this dimension.
        if (result = AddCStringToHandle("\t\tLabels: ", p->strH))
            return result;
        for(element=-1; element<dimensionSizes[dimension]; element++) {        // Loop starts from -1 because -1 returns
            if (element >= 5) {                                                // the label for the entire dimension.
                if (result = AddCStringToHandle("(and so on)", p->strH))
                    return result;
                break;
            }
            if (result = MDGetDimensionLabel(p->w, dimension, element, dimLabel))
                return result;
            sprintf(buf, "\'%s\'", dimLabel);
            if (element < dimensionSizes[dimension]-1)
                strcat(buf, ", ");
            if (result = AddCStringToHandle(buf, p->strH))
                return result;
        }
    }

    return(0);                            // XFUNC error code.
}


/*    WAFill3DWaveDirectMethod()

    This example shows how to access the data in a multi-dimensional wave
    using the direct method.

    See the top of the file for instructions on how to invoke this function
    from Igor Pro 3.0 or later.
*/

#pragma pack(2)        // All structures passed to Igor are two-byte aligned
struct WAFill3DWaveDirectMethodParams {
    waveHndl w;
    double result;
};
typedef struct WAFill3DWaveDirectMethodParams WAFill3DWaveDirectMethodParams;
#pragma pack()        // Reset structure alignment to default.

extern "C" int
WAFill3DWaveDirectMethod(WAFill3DWaveDirectMethodParams* p)
{
    waveHndl wavH;
    int waveType;
    int numDimensions;
    CountInt dimensionSizes[MAX_DIMENSIONS+1];
    char* dataStartPtr;
    /*    Pointer terminology
        dp0 points to start of double data.
        dlp points to start of double data for a layer.
        dcp points to start of double data for a column.
        dp points to a particular point of double data.
    */
    double *dp0, *dlp, *dcp, *dp;                // Pointers used for double data.
    float *fp0, *flp, *fcp, *fp;                // Pointers used for float data.
    SInt32 *lp0, *llp, *lcp, *lp;                // Pointers used for long data.
    short *sp0, *slp, *scp, *sp;                // Pointers used for short data.
    char *cp0, *clp, *ccp, *cp;                    // Pointers used for char data.
    UInt32 *ulp0, *ullp, *ulcp, *ulp;            // Pointers used for unsigned long data.
    unsigned short *usp0, *uslp, *uscp, *usp;    // Pointers used for unsigned short data.
    unsigned char *ucp0, *uclp, *uccp, *ucp;    // Pointers used for unsigned char data.
    IndexInt dataOffset;
    CountInt numRows, numColumns, numLayers;
    IndexInt row, column, layer;
    CountInt pointsPerColumn, pointsPerLayer;
    int result;

    p->result = 0;                // The Igor function result is always zero.

    wavH = p->w;
    if (wavH == NIL)
        return NOWAV;

    waveType = WaveType(wavH);
    if (waveType & NT_CMPLX)
        return NO_COMPLEX_WAVE;
    if (waveType==TEXT_WAVE_TYPE)
        return NUMERIC_ACCESS_ON_TEXT_WAVE;

    if (result = MDGetWaveDimensions(wavH, &numDimensions, dimensionSizes))
        return result;

    if (numDimensions != 3)
        return NEEDS_3D_WAVE;

    numRows = dimensionSizes[0];
    numColumns = dimensionSizes[1];
    numLayers = dimensionSizes[2];
    pointsPerColumn = numRows;
    pointsPerLayer = pointsPerColumn*numColumns;

    if (result = MDAccessNumericWaveData(wavH, kMDWaveAccessMode0, &dataOffset))
        return result;

    dataStartPtr = (char*)(*wavH) + dataOffset;

    result = 0;
    switch (waveType) {
        case NT_FP64:
            dp0 = (double*)dataStartPtr;                            // Pointer to the start of all wave data.
            for(layer=0; layer<numLayers; layer++) {
                dlp = dp0 + layer*pointsPerLayer;                    // Pointer to start of data for this layer.
                for(column=0; column<numColumns; column++) {
                    if (gCallSpinProcess && SpinProcess()) {        // Spins cursor and allows background processing.
                        result = -1;                                // User aborted.
                        break;
                    }
                    dcp = dlp + column*pointsPerColumn;                // Pointer to start of data for this column.
                    for(row=0; row<numRows; row++) {
                        dp = dcp + row;
                        *dp = (double)(row + 1000*column + 1000000*layer);
                    }
                }
                if (result != 0)
                    break;                                            // User abort.
            }
            break;

        case NT_FP32:
            fp0 = (float*)dataStartPtr;                                // Pointer to the start of all wave data.
            for(layer=0; layer<numLayers; layer++) {
                flp = fp0 + layer*pointsPerLayer;                    // Pointer to start of data for this layer.
                for(column=0; column<numColumns; column++) {
                    if (gCallSpinProcess && SpinProcess()) {        // Spins cursor and allows background processing.
                        result = -1;                                // User aborted.
                        break;
                    }
                    fcp = flp + column*pointsPerColumn;                // Pointer to start of data for this column.
                    for(row=0; row<numRows; row++) {
                        fp = fcp + row;
                        *fp = (float)(row + 1000*column + 1000000*layer);
                    }
                }
                if (result != 0)
                    break;                                            // User abort.
            }
            break;

        case NT_I32:
            lp0 = (SInt32*)dataStartPtr;                            // Pointer to the start of all wave data.
            for(layer=0; layer<numLayers; layer++) {
                llp = lp0 + layer*pointsPerLayer;                    // Pointer to start of data for this layer.
                for(column=0; column<numColumns; column++) {
                    if (gCallSpinProcess && SpinProcess()) {        // Spins cursor and allows background processing.
                        result = -1;                                // User aborted.
                        break;
                    }
                    lcp = llp + column*pointsPerColumn;                // Pointer to start of data for this column.
                    for(row=0; row<numRows; row++) {
                        lp = lcp + row;
                        *lp = (SInt32)(row + 1000*column + 1000000*layer);
                    }
                }
                if (result != 0)
                    break;                                            // User abort.
            }
            break;

        case NT_I16:
            sp0 = (short*)dataStartPtr;                                // Pointer to the start of all wave data.
            for(layer=0; layer<numLayers; layer++) {
                slp = sp0 + layer*pointsPerLayer;                    // Pointer to start of data for this layer.
                for(column=0; column<numColumns; column++) {
                    if (gCallSpinProcess && SpinProcess()) {        // Spins cursor and allows background processing.
                        result = -1;                                // User aborted.
                        break;
                    }
                    scp = slp + column*pointsPerColumn;                // Pointer to start of data for this column.
                    for(row=0; row<numRows; row++) {
                        sp = scp + row;
                        *sp = (short)(row + 1000*column + 1000000*layer);
                    }
                }
                if (result != 0)
                    break;                                            // User abort.
            }
            break;

        case NT_I8:
            cp0 = (char*)dataStartPtr;                                // Pointer to the start of all wave data.
            for(layer=0; layer<numLayers; layer++) {
                clp = cp0 + layer*pointsPerLayer;                    // Pointer to start of data for this layer.
                for(column=0; column<numColumns; column++) {
                    if (gCallSpinProcess && SpinProcess()) {        // Spins cursor and allows background processing.
                        result = -1;                                // User aborted.
                        break;
                    }
                    ccp = clp + column*pointsPerColumn;                // Pointer to start of data for this column.
                    for(row=0; row<numRows; row++) {
                        cp = ccp + row;
                        *cp = (char)(row + 1000*column + 1000000*layer);
                    }
                }
                if (result != 0)
                    break;                                            // User abort.
            }
            break;

        case NT_I32 | NT_UNSIGNED:
            ulp0 = (UInt32*)dataStartPtr;                            // Pointer to the start of all wave data.
            for(layer=0; layer<numLayers; layer++) {
                ullp = ulp0 + layer*pointsPerLayer;                    // Pointer to start of data for this layer.
                for(column=0; column<numColumns; column++) {
                    if (gCallSpinProcess && SpinProcess()) {        // Spins cursor and allows background processing.
                        result = -1;                                // User aborted.
                        break;
                    }
                    ulcp = ullp + column*pointsPerColumn;            // Pointer to start of data for this column.
                    for(row=0; row<numRows; row++) {
                        ulp = ulcp + row;
                        *ulp = (UInt32)(row + 1000*column + 1000000*layer);
                    }
                }
                if (result != 0)
                    break;                                            // User abort.
            }
            break;

        case NT_I16 | NT_UNSIGNED:
            usp0 = (unsigned short*)dataStartPtr;                    // Pointer to the start of all wave data.
            for(layer=0; layer<numLayers; layer++) {
                uslp = usp0 + layer*pointsPerLayer;                    // Pointer to start of data for this layer.
                for(column=0; column<numColumns; column++) {
                    if (gCallSpinProcess && SpinProcess()) {        // Spins cursor and allows background processing.
                        result = -1;                                // User aborted.
                        break;
                    }
                    uscp = uslp + column*pointsPerColumn;            // Pointer to start of data for this column.
                    for(row=0; row<numRows; row++) {
                        usp = uscp + row;
                        *usp = (unsigned short)(row + 1000*column + 1000000*layer);
                    }
                }
                if (result != 0)
                    break;                                            // User abort.
            }
            break;

        case NT_I8 | NT_UNSIGNED:
            ucp0 = (unsigned char*)dataStartPtr;                    // Pointer to the start of all wave data.
            for(layer=0; layer<numLayers; layer++) {
                uclp = ucp0 + layer*pointsPerLayer;                    // Pointer to start of data for this layer.
                for(column=0; column<numColumns; column++) {
                    if (gCallSpinProcess && SpinProcess()) {        // Spins cursor and allows background processing.
                        result = -1;                                // User aborted.
                        break;
                    }
                    uccp = uclp + column*pointsPerColumn;            // Pointer to start of data for this column.
                    for(row=0; row<numRows; row++) {
                        ucp = uccp + row;
                        *ucp = (unsigned char)(row + 1000*column + 1000000*layer);
                    }
                }
                if (result != 0)
                    break;                                            // User abort.
            }
            break;

        default:    // Unknown data type - possible in a future version of Igor.
            return NT_FNOT_AVAIL;
            break;
    }

    WaveHandleModified(wavH);            // Inform Igor that we have changed the wave.

    return result;
}

/*    WAFill3DWavePointMethod()

    This example shows how to access the data in a multi-dimensional wave
    using a slower but very easy access method.

    See the top of the file for instructions on how to invoke this function
    from Igor Pro 3.0 or later.

    By using the MDSetNumericWavePointValue routine to store into the wave, instead of
    accessing the wave directly, we relieve ourselves of the need to worry about
    the data type of the wave, at the cost of running more slowly.
*/

#pragma pack(2)        // All structures passed to Igor are two-byte aligned
struct WAFill3DWavePointMethodParams {
    waveHndl w;
    double result;
};
typedef struct WAFill3DWavePointMethodParams WAFill3DWavePointMethodParams;
#pragma pack()        // Reset structure alignment to default.

static int
WAFill3DWavePointMethod(WAFill3DWavePointMethodParams* p)
{
    waveHndl wavH;
    int waveType;
    int numDimensions;
    CountInt dimensionSizes[MAX_DIMENSIONS+1];
    IndexInt indices[MAX_DIMENSIONS];            // Used to pass the row, column and layer to MDSetNumericWavePointValue.
    double value[2];                            // Contains, real/imaginary parts but we use the real only.
    CountInt numRows, numColumns, numLayers;
    IndexInt row, column, layer;
    int result;

    p->result = 0;                // The Igor function result is always zero.

    wavH = p->w;
    if (wavH == NIL)
        return NOWAV;

    waveType = WaveType(wavH);
    if (waveType & NT_CMPLX)
        return NO_COMPLEX_WAVE;
    if (waveType==TEXT_WAVE_TYPE)
        return NUMERIC_ACCESS_ON_TEXT_WAVE;

    if (result = MDGetWaveDimensions(wavH, &numDimensions, dimensionSizes))
        return result;

    if (numDimensions != 3)
        return NEEDS_3D_WAVE;

    numRows = dimensionSizes[0];
    numColumns = dimensionSizes[1];
    numLayers = dimensionSizes[2];

    MemClear(indices, sizeof(indices));            // Unused indices must be zero.
    result = 0;
    for(layer=0; layer<numLayers; layer++) {
        indices[2] = layer;
        for(column=0; column<numColumns; column++) {
            if (gCallSpinProcess && SpinProcess()) {        // Spins cursor and allows background processing.
                result = -1;                                // User aborted.
                break;
            }
            indices[1] = column;
            for(row=0; row<numRows; row++) {
                indices[0] = row;
                value[0] = (double)(row + 1000*column + 1000000*layer);
                if (result = MDSetNumericWavePointValue(wavH, indices, value)) {
                    WaveHandleModified(wavH);            // Inform Igor that we have changed the wave.
                    return result;
                }
            }
        }
        if (result != 0)
            break;
    }

    WaveHandleModified(wavH);            // Inform Igor that we have changed the wave.

    return result;
}


/*    WAFill3DWaveStorageMethod()

    This example shows how to access the data in a multi-dimensional wave
    using the temp storage method. It is fast and easy but requires enough
    memory for a temporary double-precision copy of the wave data.

    See the top of the file for instructions on how to invoke this function
    from Igor Pro 3.0 or later.
*/

#pragma pack(2)        // All structures passed to Igor are two-byte aligned
struct WAFill3DWaveStorageMethodParams {
    waveHndl w;
    double result;
};
typedef struct WAFill3DWaveStorageMethodParams WAFill3DWaveStorageMethodParams;
#pragma pack()        // Reset structure alignment to default.

extern "C" int
WAFill3DWaveStorageMethod(WAFill3DWaveStorageMethodParams* p)
{
    waveHndl wavH;
    int waveType;
    int numDimensions;
    CountInt dimensionSizes[MAX_DIMENSIONS+1];
    /*    Pointer terminology
        dp0 points to start of char data.
        dlp points to start of char data for a layer.
        dcp points to start of char data for a column.
        dp points to a particular point of char data.
    */
    double *dp0, *dlp, *dcp, *dp;            // Pointers used for double data.
    CountInt numRows, numColumns, numLayers;
    IndexInt row, column, layer;
    CountInt pointsPerColumn, pointsPerLayer;
    BCInt numBytes;
    double* dPtr;
    int result, result2;

    p->result = 0;                            // The Igor function result is always zero.

    wavH = p->w;
    if (wavH == NIL)
        return NOWAV;

    waveType = WaveType(wavH);
    if (waveType & NT_CMPLX)
        return NO_COMPLEX_WAVE;
    if (waveType==TEXT_WAVE_TYPE)
        return NUMERIC_ACCESS_ON_TEXT_WAVE;

    if (result = MDGetWaveDimensions(wavH, &numDimensions, dimensionSizes))
        return result;

    if (numDimensions != 3)
        return NEEDS_3D_WAVE;

    numRows = dimensionSizes[0];
    numColumns = dimensionSizes[1];
    numLayers = dimensionSizes[2];
    pointsPerColumn = numRows;
    pointsPerLayer = pointsPerColumn*numColumns;

    numBytes = WavePoints(wavH) * sizeof(double);            // Bytes needed for copy
    //    This example doesn't support complex waves.
    //    if (isComplex)
    //        numBytes *= 2;
    dPtr = (double*)NewPtr(numBytes);
    if (dPtr==NIL)
        return NOMEM;

    if (result = MDGetDPDataFromNumericWave(wavH, dPtr)) {    // Get a copy of the wave data.
        DisposePtr((Ptr)dPtr);
        return result;
    }

    result = 0;
    dp0 = dPtr;                                                // Pointer to the start of all wave data.
    for(layer=0; layer<numLayers; layer++) {
        dlp = dp0 + layer*pointsPerLayer;                    // Pointer to start of data for this layer.
        for(column=0; column<numColumns; column++) {
            if (gCallSpinProcess && SpinProcess()) {        // Spins cursor and allows background processing.
                result = -1;                                // User aborted.
                break;
            }
            dcp = dlp + column*pointsPerColumn;                // Pointer to start of data for this column.
            for(row=0; row<numRows; row++) {
                dp = dcp + row;
                *dp = (double)(row + 1000*column + 1000000*layer);
            }
        }
        if (result != 0)
            break;
    }

    if (result2 = MDStoreDPDataInNumericWave(wavH, dPtr)) {    // Store copy in the wave.
        DisposePtr((Ptr)dPtr);
        return result2;
    }

    DisposePtr((Ptr)dPtr);
    WaveHandleModified(wavH);            // Inform Igor that we have changed the wave.

    return result;
}

static int
DoAppendAndPrepend(Handle textH, Handle prependStringH, Handle appendStringH)
{
    BCInt textHLength;
    BCInt appendStringHLength;
    BCInt prependStringHLength;

    textHLength = GetHandleSize(textH);
    prependStringHLength = GetHandleSize(prependStringH);
    appendStringHLength = GetHandleSize(appendStringH);

    SetHandleSize(textH, textHLength + prependStringHLength + appendStringHLength);
    if (MemError())
        return NOMEM;
    memmove(*textH+prependStringHLength, *textH, textHLength);                                // Make room for prependString.
    memcpy(*textH, *prependStringH, prependStringHLength);                                    // Prepend prependString.
    memcpy(*textH+textHLength+prependStringHLength, *appendStringH, appendStringHLength);    // Append appendString.
    return 0;
}

/*    WAModifyTextWave()

    This example shows how to access the data in a multi-dimensional text wave.

    See the top of the file for instructions on how to invoke this function
    from Igor Pro 3.0 or later.
*/

#pragma pack(2)        // All structures passed to Igor are two-byte aligned
struct WAModifyTextWaveParams {
        Handle appendStringH;            // String to be appended to each wave point.
        Handle prependStringH;            // String to be prepended to each wave point.
        waveHndl w;
        double result;
};
typedef struct WAModifyTextWaveParams WAModifyTextWaveParams;
#pragma pack()        // Reset structure alignment to default.

extern "C" int
WAModifyTextWave(WAModifyTextWaveParams* p)
{
    waveHndl wavH;
    int waveType;
    int numDimensions;
    CountInt dimensionSizes[MAX_DIMENSIONS+1];
    IndexInt indices[MAX_DIMENSIONS];                // Used to pass the row, column and layer to MDSetTextWavePointValue.
    CountInt numRows, numColumns, numLayers, numChunks;
    IndexInt row, column, layer, chunk;
    Handle textH;
    int result;

    result = 0;

    textH = NewHandle(0L);            // Handle used to pass text wave characters to Igor.
    if (textH == NIL) {
        result = NOMEM;
        goto done;
    }

    if (p->prependStringH == NIL) {
        result = USING_NULL_STRVAR;        // The user called the function with an uninitialized string variable.
        goto done;
    }

    if (p->appendStringH == NIL) {
        result = USING_NULL_STRVAR;        // The user called the function with an uninitialized string variable.
        goto done;
    }

    wavH = p->w;
    if (wavH == NIL) {
        result = NOWAV;                    // The user called the function with a missing wave or uninitialized wave reference variable.
        goto done;
    }

    waveType = WaveType(wavH);
    if (waveType!=TEXT_WAVE_TYPE) {
        result = TEXT_ACCESS_ON_NUMERIC_WAVE;
        goto done;
    }

    if (result = MDGetWaveDimensions(wavH, &numDimensions, dimensionSizes))
        goto done;

    numRows = dimensionSizes[0];
    numColumns = dimensionSizes[1];
    if (numColumns==0)
        numColumns = 1;
    numLayers = dimensionSizes[2];
    if (numLayers==0)
        numLayers = 1;
    numChunks = dimensionSizes[3];
    if (numChunks==0)
        numChunks = 1;

    MemClear(indices, sizeof(indices));            // Clear unused indices.
    result = 0;
    for(chunk=0; chunk<numChunks; chunk++) {
        indices[3] = chunk;
        for(layer=0; layer<numLayers; layer++) {
            indices[2] = layer;
            for(column=0; column<numColumns; column++) {
                if (gCallSpinProcess && SpinProcess()) {        // Spins cursor and allows background processing.
                    result = -1;                                // User aborted.
                    break;
                }
                indices[1] = column;
                for(row=0; row<numRows; row++) {
                    indices[0] = row;
                    if (result = MDGetTextWavePointValue(wavH, indices, textH))
                        goto done;
                    if (result = DoAppendAndPrepend(textH, p->prependStringH, p->appendStringH))
                        goto done;
                    if (result = MDSetTextWavePointValue(wavH, indices, textH))
                        goto done;
                }
            }
            if (result != 0)
                break;
        }
        if (result != 0)
            break;
    }

done:
    if (wavH != NIL)
        WaveHandleModified(wavH);                // Inform Igor that we have changed the wave.
    if (textH != NIL)
        DisposeHandle(textH);
    if (p->prependStringH)
        DisposeHandle(p->prependStringH);        // We need to get rid of input parameters.
    if (p->appendStringH)
        DisposeHandle(p->appendStringH);        // We need to get rid of input parameters.
    p->result = 0;                                // The Igor function result is always zero.

    return(result);
}

