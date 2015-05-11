#define ENERGY_COMPONENT_WIRE_ACTIVE 0
#define ENERGY_COMPONENT_WIRE_INACTIVE 1
#define ENERGY_COMPONENT_WIRE_LEAK 2
#define ENERGY_COMPONENT_IMEM_ACTIVE_READ 3
#define ENERGY_COMPONENT_IMEM_INACTIVE_READ 4
#define ENERGY_COMPONENT_IMEM_LEAK 5
#define ENERGY_COMPONENT_CLOCK_ACTIVE 6
#define ENERGY_COMPONENT_CLOCK_LEAK 7
#define ENERGY_COMPONENT_LUT_ACTIVE 8
#define ENERGY_COMPONENT_LUT_INACTIVE 9
#define ENERGY_COMPONENT_DATA_WRITE_ACTIVE 10
#define ENERGY_COMPONENT_DATA_WRITE_INACTIVE 11
#define ENERGY_COMPONENT_PE_LEAK 12
#define ENERGY_COMPONENTS 13


double calc_energy(struct energy_model *model, // the energy model
		   int used_waves, // number of waves actually used (may be less than what substrate supports)
		   int *gused, // growth schedule used per wave, also may be less than substrate design
		   int *used_up_wires, // up wires used at each height
		   int *used_down_wires, // down wires used at each height
		   int used_luts, // number of LUT evaluations
		   int used_pe_inputs, // number of inputs stored into PE
		   int height, // height of tree
		   double *energy_components

);
		   
