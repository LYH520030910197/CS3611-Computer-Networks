#!/usr/bin/python3

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.log import setLogLevel
from mininet.cli import CLI


class NetworkTopo(Topo):
    r"""
    A network containing 3 switches and 4 hosts.
          h2              h1
             \          /
               s1 -- s3
             /          \
    h4 -- s2              h3
    """

    def build(self):
        s1, s2, s3 = [self.addSwitch(s) for s in ('s1', 's2', 's3')]
        h1, h2, h3, h4 = [self.addHost(h) for h in ('h1', 'h2', 'h3', 'h4')]

        self.addLink(s1, s2, bw=10, loss=30) # Set the loss rate here
        self.addLink(s1, s3, bw=10)
        for s, h in [(s1, h2), (s2, h4), (s3, h3), (s3, h1)]:
            self.addLink(s, h)


def run():
    "Create network and run command line interface"
    topo = NetworkTopo()
    net = Mininet(topo=topo,
                  host=CPULimitedHost, link=TCLink,
                  autoStaticArp=True)
    net.start()

    # Running command line interface
    CLI(net)
    net.stop()


if __name__ == '__main__':
    # Tell mininet to print useful information
    setLogLevel('info')
    run()
