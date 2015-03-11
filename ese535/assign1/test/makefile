# makefile to run some sched_main tests

BLIF = e64.blif s1423.blif example.blif

assign1: $(BLIF:.blif=.sched) 

clean:
	rm -f *.place *.net *.echo *.stackdump *.core *.sched


%.sched:	%.blif
	../sched_main $*.blif $*.net $*.place dummy.arch $*.sched \
        -lut_size 4 -cluster_size 4 \
        -input_cluster_size 4 -inputs_per_cluster 16 \
        -clocks_per_cluster 1 -global_clocks on \
        -approach 1 \
        -sample_boolean_flag off  -sample_float_flag 0.9 \
        -block_delay 0.1 \
        -intra_cluster_net_delay 0.1 \
        -inter_cluster_initial_net_delay 0.5 \
        -inter_cluster_per_manhattan_hop_delay 0.1 \
        -intra_cluster_net_energy 0.02 \
        -inter_cluster_initial_net_energy 0.03 \
        -inter_cluster_per_manhattan_hop_energy 0.04 


