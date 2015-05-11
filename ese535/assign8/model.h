struct energy_model {
  // each of these int * variables are arrays
  //  where the [i] element is the energy at height i in the tree
  //  i=0 is the wires in/out of PE, 
  //  i>0 is the wires in/out of top of switches at that level
  int *gphysical; // this is the physical growth level; saved here so can use in calculations
  double *wire_active; // energy per used wire cycle for each wire switched, including switches and buffers
  double *wire_inactive; // energy for a wire on a cycle it is not used (maybe 0?)
  double *wire_leak; // energy leaked per wire per cycle due to its swiches and buffers
  double *imem_active_read; // energy reading from imem controlling a switch port at specified level when activating wire (per switch port)
  double *imem_inactive_read; // energy reading from imem when not activating associated wire (per switch port)
  double *imem_leak; // energy leaked per from Imem memory per wire (per switch)
  double *clock_active; // energy spent on clock at height (all subtrees at that height)
  double *clock_leak; // leakage from clock buffers on a cycle when a height isn't clocked (all subtrees at that height)
  double lut_active; // energy for LUT (including instruction and data memory) when activated
  double lut_inactive; // energy for LUT (including instruction and data memory) when not activated
  double data_write_active; // energy per write of data into memory
  double data_write_inactive; // energy per potential write port per cycle when no write into memory
  double pe_leak; // energy leaked per PE per cycle
};


struct energy_model *model(int *gphysical, // growth for physical structure gphysical[0] is leaf, rest are 1 or 2 (or 0 at root) for 1:1, 2:1, wire
			   int *gwave, // similar, but this is logical -- when larger, there is multiplexing
			   int waves,  // number of routing waves, including CF effects
			   int luts_per_pe,  // S
			   int h // height of physical tree
			   );
// we can have one of these for each of the PE (or other) variants:

struct energy_model *model_flat(int *gphysical, int *gwave, int waves, int luts_per_pe, int h);
struct energy_model *model_sparse_lut(int *gphysical, int *gwave, int waves, int luts_per_pe, int h);
struct energy_model *model_sparse_input_lut(int *gphysical, int *gwave, int waves, int luts_per_pe, int h);

void free_model(struct energy_model *model);
