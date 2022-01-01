#!/bin/sh

sudo ovs-ofctl add-flow s1 "in_port=s1-eth1 actions=output:s1-eth3"
sudo ovs-ofctl add-flow s1 "in_port=s1-eth2 actions=output:s1-eth3"
sudo ovs-ofctl add-flow s2 "in_port=s2-eth1 actions=output:s2-eth2"
sudo ovs-ofctl add-flow s2 "in_port=s2-eth3 actions=output:s2-eth2"
