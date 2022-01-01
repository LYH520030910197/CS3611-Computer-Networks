#!/usr/bin/python3

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.log import setLogLevel
from mininet.cli import CLI


class NetworkTopo(Topo):
    r"""
    A network containing 3 switches and 4 hosts.
          h2              h1
             \          /
               s1 -- s3
             /     /    \
    h4 -- s2 -----        h3
    """

    def build(self):
        s1, s2, s3 = [self.addSwitch(s) for s in ('s1', 's2', 's3')]
        h1, h2, h3, h4 = [self.addHost(h) for h in ('h1', 'h2', 'h3', 'h4')]

        for s1, s in [(s1, s2), (s1, s3)]:
            self.addLink(s1, s)
        for s, h in [(s1, h2), (s2, h4), (s3, h3), (s3, h1)]:
            self.addLink(s, h)
        self.addLink(s2, s3)


def run():
    "Create network and run command line interface"
    topo = NetworkTopo()
    net = Mininet(topo)
    net.start()

    # Running command line interface (pingall)
    CLI(net)
    net.stop()


if __name__ == '__main__':
    # Tell mininet to print useful information
    setLogLevel('info')
    run()
