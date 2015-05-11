#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "model.h"
#include "energy.h"

/* 2/28/15 -- this is a placeholder model, developed for initial debug; it will be superceded by a better model */

#define LEAK_UNIT 0.001
#define INACTIVE_UNIT 0.1
#define ACTIVE_UNIT 1
#define PE_WIDTH 10

struct energy_model *model(int *gphysical, // growth for physical structure gphysical[0] is leaf, rest are 1 or 2 (or 0 at root) for 1:1, 2:1, wire
			   int *gwave, // similar, but this is logical -- when larger, there is multiplexing
			   int waves,  // number of routing waves, including CF effects
			   int luts_per_pe,  // S
			   int height // height of physical tree
			   )
{

  struct energy_model *result=(struct energy_model *)malloc(sizeof(struct energy_model));
  result->gphysical=(int *)malloc(sizeof(double)*height);
  result->wire_active=(double *)malloc(sizeof(double)*height);
  result->wire_inactive=(double *)malloc(sizeof(double)*height);
  result->wire_leak=(double *)malloc(sizeof(double)*height);
  result->imem_active_read=(double *)malloc(sizeof(double)*height);
  result->imem_inactive_read=(double *)malloc(sizeof(double)*height);
  result->imem_leak=(double *)malloc(sizeof(double)*height);
  result->clock_leak=(double *)malloc(sizeof(double)*height);
  result->clock_active=(double *)malloc(sizeof(double)*height);

  // basic scalars
  result->lut_active=16*ACTIVE_UNIT; // very crude; should depend on PE Imem and data depth
  result->lut_inactive=INACTIVE_UNIT; // could be 0 in some models
  result->data_write_active=ACTIVE_UNIT; // also should depend on data depth
  result->data_write_inactive=INACTIVE_UNIT; // likely to require some logic and memory 
  result->pe_leak=LEAK_UNIT*32*waves; // based on number of bits; should depend on PE Imem and data depth

  int h;
  int ch=1;
  int pch=1;
  for (h=0;h<height;h++)
    {
      result->gphysical[h]=gphysical[h]; // save away gphysical
      ch=gwave[h]*ch;
      pch=result->gphysical[h]*pch;
      int imem_depth=(ch/pch)*waves;

      // set wires and imems in tree
      result->imem_leak[h]=LEAK_UNIT*3*imem_depth; // based on number of bits
      result->imem_inactive_read[h]=0; // maybe optimistic
      result->imem_active_read[h]=3*sqrt(3.0*(double)imem_depth)*ACTIVE_UNIT; // basic scaling
      result->wire_leak[h]=LEAK_UNIT; // just for buffer (probably should be buffered differently per level)
      result->wire_inactive[h]=0; // achievable with appropriate architecture
      result->wire_active[h]=pow(2,ceil(((double)h)/2.0))*PE_WIDTH*ACTIVE_UNIT; // oversimplified

      // set clock in tree
      result->clock_leak[h]=LEAK_UNIT; // probably buffered differently by tree level;
      result->clock_active[h]=2*pow(2,ceil(((double)h)/2.0))*PE_WIDTH*ACTIVE_UNIT; // probably larger due to various registers
    }

  return(result);

}

void free_model(struct energy_model *model)
{
  // free arrays first
  free(model->clock_active);
  free(model->clock_leak);
  free(model->wire_active);
  free(model->wire_inactive);
  free(model->wire_leak);
  free(model->imem_active_read);
  free(model->imem_inactive_read);
  free(model->imem_leak);
  free(model->gphysical);
  // then can free structure
  free(model);
}
