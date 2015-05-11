#include <stdio.h>
#include <string.h>
#include <math.h>
#include "model.h"
#include "energy.h"

double calc_energy(struct energy_model *model, // the energy model
		   int used_waves, // number of waves actually used (may be less than what substrate supports)
		   int *gused, // growth schedule used per wave, also may be less than substrate design
		   int *used_up_wires, // up wires used at each height
		   int *used_down_wires, // down wires used at each height
		   int used_luts, // number of LUT evaluations
		   int used_pe_inputs, // number of inputs stored into PE
		   int height, // height of tree
		   double *energy_components
		   
)
{

  double energy=0;
  double energy_wire_active=0.0;
  double energy_wire_inactive=0.0;
  double energy_wire_leak=0.0;
  double energy_imem_active_read=0.0;
  double energy_imem_inactive_read=0.0;
  double energy_imem_leak=0.0;
  double energy_clock_active=0.0;
  double energy_clock_leak=0.0;
  double energy_lut_active=0.0;
  double energy_lut_inactive=0.0;
  double energy_data_write_active=0.0;
  double energy_data_write_inactive=0.0;
  double energy_pe_leak=0.0;

  int h;

  int ch=gused[0];
  int pch=model->gphysical[0];
  int cycles_per_wave=((ch+pch-1)/pch);
  for (h=1;h<=height;h++)
    {
      ch=gused[h]*ch;
      pch=model->gphysical[h]*pch;
      cycles_per_wave+=((ch+pch-1)/pch);
    }
  int cycles_per_eval=cycles_per_wave*2*used_waves;

  // everything leaks, all the time
  energy_pe_leak=model->pe_leak*pow(2,height)*cycles_per_eval;
  energy_clock_leak=model->clock_leak[0]*cycles_per_eval;
  energy_wire_leak=model->pe_leak*pow(2,height)*model->wire_leak[0]*cycles_per_eval*2; // x2 for up and down
  energy_imem_leak=model->pe_leak*pow(2,height)*model->imem_leak[0]*cycles_per_eval*2; // x2 for up and down
  ch=gused[0];
  pch=model->gphysical[0];
  for (h=1;h<height;h++)
    {
      int subtrees=pow(2,height-h);
      pch=model->gphysical[h]*pch;
      int switches=subtrees*pch;

      energy_clock_leak+=model->clock_leak[h]*cycles_per_eval;
      energy_wire_leak+=switches*model->wire_leak[h]*cycles_per_eval*2; // x2 for up and down
      energy_imem_leak+=switches*model->imem_leak[h]*cycles_per_eval*2; // x2 for up and down
    }
  
  // active and inactive

  // start with PE
  energy_data_write_active=used_pe_inputs*model->data_write_active;
  energy_data_write_inactive=(pow(2,height)*cycles_per_eval*gused[0]-used_pe_inputs)*model->data_write_inactive;
  energy_lut_active=used_luts*model->lut_active;
  energy_lut_inactive=(pow(2,height)*cycles_per_eval-used_luts)*model->lut_inactive;

  // now wires and switches
  ch=1;
  pch=1;
  for (h=0;h<height;h++)
    {
      int subtrees=pow(2,height-h);
      ch=gused[h]*ch;
      pch=model->gphysical[h]*pch;
      int switches=subtrees*pch*used_waves;

      // wires
      energy_wire_active+=(used_up_wires[h]+used_down_wires[h])*model->wire_active[h];
      energy_wire_inactive+=(switches*2-used_up_wires[h]-used_down_wires[h])*model->wire_inactive[h];//*2 for up and down
      printf("energy: h=%d sw=%d uw=%d,%d wire_active=%f total_active=%f\n",h,switches,used_up_wires[h],used_down_wires[h],model->wire_active[h],
	     energy_wire_active);
      printf("energy: h=%d sw=%d uw=%d,%d inactive wires=%d, wire_inactive=%f total_inactive=%f\n",h,switches,used_up_wires[h],used_down_wires[h],
	     (switches*2-used_up_wires[h]-used_down_wires[h]),
	     model->wire_inactive[h],
	     energy_wire_inactive);
      if ((switches*2-used_up_wires[h]-used_down_wires[h])<0)
	{
	  fprintf(stderr,"WARNING: negative inactive switches %d, waves=%d, switches=%d, uw=(%d,%d)\n",
		  (switches*2-used_up_wires[h]-used_down_wires[h]),used_waves,switches,used_up_wires[h],used_down_wires[h]);
	}

      // imems
      energy_imem_active_read+=used_up_wires[h]*model->imem_active_read[h]; // up link
      energy_imem_inactive_read+=(switches-used_up_wires[h])*model->imem_inactive_read[h];
      if (used_down_wires[h]>0)
	{
	  energy_imem_active_read+=used_down_wires[h]*model->imem_active_read[h+1]; // down link
	  energy_imem_inactive_read+=(switches-used_down_wires[h])*model->imem_inactive_read[h+1];
	}

    }

  if (energy_components!=(double *)NULL)
    {
      // stick into energy_components
      energy_components[ENERGY_COMPONENT_WIRE_ACTIVE]= energy_wire_active;
      energy_components[ENERGY_COMPONENT_WIRE_INACTIVE]= energy_wire_inactive;
      energy_components[ENERGY_COMPONENT_WIRE_LEAK]= energy_wire_leak;
      energy_components[ENERGY_COMPONENT_IMEM_ACTIVE_READ]= energy_imem_active_read;
      energy_components[ENERGY_COMPONENT_IMEM_INACTIVE_READ]= energy_imem_inactive_read;
      energy_components[ENERGY_COMPONENT_IMEM_LEAK]= energy_imem_leak;
      energy_components[ENERGY_COMPONENT_CLOCK_ACTIVE]= energy_clock_active;
      energy_components[ENERGY_COMPONENT_CLOCK_LEAK]= energy_clock_leak;
      energy_components[ENERGY_COMPONENT_LUT_ACTIVE]= energy_lut_active;
      energy_components[ENERGY_COMPONENT_LUT_INACTIVE]= energy_lut_inactive;
      energy_components[ENERGY_COMPONENT_DATA_WRITE_ACTIVE]= energy_data_write_active;
      energy_components[ENERGY_COMPONENT_DATA_WRITE_INACTIVE]= energy_data_write_inactive;
      energy_components[ENERGY_COMPONENT_PE_LEAK]= energy_pe_leak;
    }

  // add up energy components
  energy=energy_wire_active
    + energy_wire_inactive
    + energy_wire_leak
    + energy_imem_active_read
    + energy_imem_inactive_read
    + energy_imem_leak
    + energy_clock_active
    + energy_clock_leak
    + energy_lut_active
    + energy_lut_inactive
    + energy_data_write_active
    + energy_data_write_inactive
    + energy_pe_leak;

  return(energy);

}
		   
