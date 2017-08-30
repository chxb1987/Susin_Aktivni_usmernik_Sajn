/****************************************************************
* FILENAME:     REP_REG.c
* DESCRIPTION:  Repetitive controller (regulator) which is reducing periodic disturbance
* AUTHOR:       Denis Su�in
* START DATE:   6.4.2016
* VERSION:      1.0
*
* CHANGES :
* VERSION   DATE        WHO             DETAIL
* 1.0       6.4.2016   Denis Su�in      Initial version
* 1.1		21.8.2017  Denis Su�in		Corrections of comments and names of variables
****************************************************************/

#include "REP_REG.h"

// globalne spremenljivke






// funkcija
#pragma CODE_SECTION(REP_REG, "ramfuncs");
void REP_REG (REP_REG_float *v)
{
    // lokalne spremenljivke






    // program

    // omejitev bufferja
    if (v->BufferHistoryLength > MAX_LENGTH_REP_REG_BUFFER)
    {
        v->BufferHistoryLength = MAX_LENGTH_REP_REG_BUFFER;
    }
    else if (v->BufferHistoryLength < 1)
    {
        v->BufferHistoryLength = 1;
    }




    // izra�un trenutnega indeksa bufferja
    v->i = (int)(v->SamplingSignal*v->BufferHistoryLength);



	
	/* circular buffer */
	
    // �e se indeks spremeni, potem gre algoritem dalje (vsako periodo signala, ne pa vsako vzor�no periodo/interval)
    if ((v->i != v->i_prev) || (v->i == 0 && v->i_prev == 0))
    {
        // manipuliranje z indeksi - zaradi circular bufferja
        if (v->i > v->i_prev)
        {
            // indeks, ki ka�e v prihodnost (potrebujem za kompenzacijo zakasnitve)
            v->index = v->i + v->k;
            // to�ke okoli i (potrebujem za FIR filter)
            v->i_plus_one = v->i + 1;
            v->i_minus_one = v->i - 1;
            v->i_plus_two = v->i + 2;
            v->i_minus_two = v->i - 2;

            // omejitve zaradi circular bufferja
            if (v->index > (v->BufferHistoryLength - 1))
            {
                v->index = v->index - v->BufferHistoryLength;
            }

            if (v->i_plus_one > (v->BufferHistoryLength - 1))
            {
                v->i_plus_one = v->i_plus_one - v->BufferHistoryLength;
            }

            if (v->i_plus_two > (v->BufferHistoryLength - 1))
            {
                v->i_plus_two = v->i_plus_two - v->BufferHistoryLength;
            }

            if (v->i_minus_one < 0)
            {
                v->i_minus_one = v->i_minus_one + v->BufferHistoryLength;
            }

            if (v->i_minus_two < 0)
            {
                v->i_minus_two = v->i_minus_two + v->BufferHistoryLength;
            }

        } // end of if (v->i > v->i_prev)
        else if (v->i < v->i_prev)
        {
            // indeks, ki ka�e v prihodnost (potrebujem za kompenzacijo zakasnitve)
            v->index = v->i - v->k;
            // to�ke okoli i (potrebujem za FIR filter)
            v->i_plus_one = v->i - 1;
            v->i_minus_one = v->i + 1;
            v->i_plus_two = v->i - 2;
            v->i_minus_two = v->i + 2;

            // omejitve zaradi circular bufferja
            if (v->index < 0)
            {
                v->index = v->index + v->BufferHistoryLength;
            }

            if (v->i_plus_one < 0)
            {
                v->i_plus_one = v->i_plus_one + v->BufferHistoryLength;
            }

            if (v->i_plus_two < 0)
            {
                v->i_plus_two = v->i_plus_two + v->BufferHistoryLength;
            }

            if (v->i_minus_one > (v->BufferHistoryLength - 1))
            {
                v->i_minus_one = v->i_minus_one - v->BufferHistoryLength;
            }

            if (v->i_minus_two > (v->BufferHistoryLength - 1))
            {
                v->i_minus_two = v->i_minus_two - v->BufferHistoryLength;
            }
        } // end of else if (v->i < v->i_prev)




        /* koda repetitivnega regulatorja */

        // izra�unam trenutni error
        v->Err = v->Ref - v->Fdb;

        // izra�unam novi akumuliran error (indeks i je zaenkrat �e i - N, ker �e ni prepisan)
        v->ErrSum = v->Krep * v->Err +
                    v->w0 * v->ErrSumHistory[v->i] +
                    v->w1 * v->ErrSumHistory[v->i_plus_one] +
                    v->w1 * v->ErrSumHistory[v->i_minus_one] +
                    v->w2 * v->ErrSumHistory[v->i_plus_two] +
                    v->w2 * v->ErrSumHistory[v->i_minus_two];

        // omejim trenutni error, da ne gre v nasi�enje
        v->ErrSum = (v->ErrSum > v->ErrSumMax)? v->ErrSumMax: v->ErrSum;
        v->ErrSum = (v->ErrSum < v->ErrSumMin)? v->ErrSumMin: v->ErrSum;

        // zapi�em trenutni akumuliran error v buffer na trenutno mesto (v naslednjem ciklu bo to error v prej�njem ciklu)
        v->ErrSumHistory[v->i] = v->ErrSum;

        // izra�unam korekcijo s pomo�jo zgodovine (upo�tevana kompenzacija zakasnitve)
        v->Correction = v->ErrSumHistory[v->index];

        // izra�unam izhod
        v->Out = v->Correction;

        // shranim vrednost indeksa i, ki bo v naslednjem ciklu prej�nji i
        v->i_prev = v->i;




    } // end of if (i != i_prev)



		
    // omejim izhod
    v->Out = (v->Out > v->OutMax)? v->OutMax: v->Out;
    v->Out = (v->Out < v->OutMin)? v->OutMin: v->Out;




} // konec funkcije
