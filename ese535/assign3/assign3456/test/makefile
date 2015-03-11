# makefile to run some sched_main tests

BLIF = example.blif e64.blif s1423.blif alu4.blif frisc.blif blob_merge.blif bgm.blif diffeq1.blif diffeq2.blif sha.blif stereovision0.blif stereovision1.blif 

assign3: $(BLIF:.blif=.cost3) 

test1: $(BLIF:.blif=.cost1) 
test2: $(BLIF:.blif=.cost2) 
test3: $(BLIF:.blif=.cost3) 
test4: $(BLIF:.blif=.cost4) 
test5: $(BLIF:.blif=.cost5) 
test6: $(BLIF:.blif=.cost6) 

clean:
	rm -f *.place* *.net *.echo *.stackdump *.core *.sched* *.cost* *.rinfo

%.cost1:	%.blif
	../main $*.blif $*.net $*.place1 dummy.arch $*.sched1 $*.cost1 \
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

%.cost3:	%.blif
	../main $*.blif $*.net $*.place3 dummy.arch $*.sched3 $*.cost3 \
        -lut_size 4 -cluster_size 4 \
        -input_cluster_size 4 -inputs_per_cluster 16 \
        -clocks_per_cluster 1 -global_clocks on \
        -approach 3 \
        -sample_boolean_flag off  -sample_float_flag 0.9 \
        -block_delay 0.1 \
        -intra_cluster_net_delay 0.1 \
        -inter_cluster_initial_net_delay 0.5 \
        -inter_cluster_per_manhattan_hop_delay 0.1 \
        -intra_cluster_net_energy 0.02 \
        -inter_cluster_initial_net_energy 0.03 \
        -inter_cluster_per_manhattan_hop_energy 0.04 


%.cost4:	%.blif
	../main $*.blif $*.net $*.place4 dummy.arch $*.sched4 $*.cost4 \
        -lut_size 4 -cluster_size 4 \
        -input_cluster_size 4 -inputs_per_cluster 16 \
        -clocks_per_cluster 1 -global_clocks on \
        -approach 4 \
        -sample_boolean_flag off  -sample_float_flag 0.9 \
        -block_delay 0.1 \
        -intra_cluster_net_delay 0.1 \
        -inter_cluster_initial_net_delay 0.5 \
        -inter_cluster_per_manhattan_hop_delay 0.1 \
        -intra_cluster_net_energy 0.02 \
        -inter_cluster_initial_net_energy 0.03 \
        -inter_cluster_per_manhattan_hop_energy 0.04 

%.cost5:	%.blif
	../main $*.blif $*.net $*.place5 dummy.arch $*.sched5 $*.cost5 \
        -lut_size 4 -cluster_size 4 \
        -input_cluster_size 4 -inputs_per_cluster 16 \
        -clocks_per_cluster 1 -global_clocks on \
        -approach 5 \
        -sample_boolean_flag off  -sample_float_flag 0.9 \
        -block_delay 0.1 \
        -intra_cluster_net_delay 0.1 \
        -inter_cluster_initial_net_delay 0.5 \
        -inter_cluster_per_manhattan_hop_delay 0.1 \
        -intra_cluster_net_energy 0.02 \
        -inter_cluster_initial_net_energy 0.03 \
        -inter_cluster_per_manhattan_hop_energy 0.04 

%.cost6:	%.blif
	../main $*.blif $*.net $*.place6 dummy.arch $*.sched6 $*.cost6 \
        -lut_size 4 -cluster_size 4 \
        -input_cluster_size 4 -inputs_per_cluster 16 \
        -clocks_per_cluster 1 -global_clocks on \
        -approach 6 \
        -sample_boolean_flag off  -sample_float_flag 0.9 \
        -block_delay 0.1 \
        -intra_cluster_net_delay 0.1 \
        -inter_cluster_initial_net_delay 0.5 \
        -inter_cluster_per_manhattan_hop_delay 0.1 \
        -intra_cluster_net_energy 0.02 \
        -inter_cluster_initial_net_energy 0.03 \
        -inter_cluster_per_manhattan_hop_energy 0.04 
