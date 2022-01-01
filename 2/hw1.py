#!/usr/bin/python3

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.log import setLogLevel


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

        for s1, s in [(s1, s2), (s1, s3)]:
            # 10 Mbps
            self.addLink(s1, s, bw=10)
        for s, h in [(s1, h2), (s2, h4), (s3, h3), (s3, h1)]:
            self.addLink(s, h)


def perfTest():
    "Create network and run simple performance test"
    topo = NetworkTopo()
    net = Mininet(topo=topo,
                  host=CPULimitedHost, link=TCLink,
                  autoStaticArp=True)
    net.start()

    # Testing bandwidth between h1 and other hosts
    h1, h2, h3, h4 = net.getNodeByName('h1', 'h2', 'h3', 'h4')
    for pair in [(h1, h2), (h1, h3), (h1, h4)]:
        net.iperf(pair)
    net.stop()


if __name__ == '__main__':
    # Tell mininet to print useful information
    setLogLevel('info')
    perfTest()
