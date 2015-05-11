#include <stdio.h>
#include <string.h>
#include "util.h"
#include "model.h"
#include "energy.h"
#include "tree.h"
#include "tree_energy.h"

int get_used_luts() // includes inputs and outputs
{
  int i;
  int res=0;
  int tree_size=get_size();
  for (i=0;i<tree_size;i++)
    res+=tree_used_slots(i);
  return(res);
}

int get_used_pe_inputs(int cluster_size, int inputs_per_lut) 
{
  int i;
  int res=0;
  int tree_size=get_size();
  for (i=0;i<tree_size;i++)
    res+=cluster_io(i,cluster_size,inputs_per_lut);
  return(res);
}


void calculate_and_print_energy(char *source_file, char *cost_file, int makespan, int maxwaves, int height, int cluster_size, int inputs_per_lut,int *gwave, int *pwave)
{

 FILE *fp; 
 int i;

 fp = my_fopen(cost_file,"w",0);

 int used_waves=get_max_domain_used(height-1,maxwaves)+1;
 int *used_up_wires=get_used_wires(height-1,TRUE,maxwaves);
 int *used_down_wires=get_used_wires(height-1,FALSE,maxwaves);
 int used_pe_inputs=get_used_pe_inputs(cluster_size,inputs_per_lut);
 int used_luts=get_used_luts();
 double *energy_components=(double *)malloc(sizeof(double)*ENERGY_COMPONENTS);
  
  struct energy_model *emodel=model(pwave, // physical
				    gwave, // logical
				    used_waves,  // number of routing waves
				    cluster_size,  // S
				    height // need one more here for 0 to height
				    );


  double energy=calc_energy(emodel, // the energy model
			    used_waves, // number of waves actually used (may be less than what substrate supports)
			    gwave, // growth schedule used per wave, also may be less than substrate design
			    used_up_wires, // up wires used at each height
			    used_down_wires, // down wires used at each height
			    used_luts, // number of LUT evaluations
			    used_pe_inputs, // number of inputs stored into PE
			    (height-1), // height of tree
			    energy_components
			    );

  fprintf(fp,"gwave:");
  if (gwave!=(int *)NULL)
    for (i=0;i<height;i++)
      fprintf(fp,"%d ",gwave[i]);
  fprintf(fp,"\n");

  fprintf(fp,"pwave:");
  if (gwave!=(int *)NULL)
    for (i=0;i<height;i++)
      fprintf(fp,"%d ",pwave[i]);
  fprintf(fp,"\n");

  float context_factor = (float)used_waves/(float)makespan;

  fprintf(fp,"components");
  for (i=0;i<ENERGY_COMPONENTS;i++)
    fprintf(fp,",%f",energy_components[i]);
  fprintf(fp,"\n");

  fprintf(fp,"%s, %d, %d, %3.2f, %f\n",source_file,makespan,used_waves,context_factor,energy);
  


}
