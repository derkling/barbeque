#!/usr/bin/python 
#
#       @file  bbqProfileGrapher.py
#      @brief  Barbeque RTRM Profiling and Graphinc application
#
# This is a simple script which produces a scheduling graph out of a Barbeque RTRM
# log file.
#
#     @author  Patrick Bellasi (derkling), derkling@google.com
#
#   @internal
#     Created  09/07/2011
#    Revision  $Id: doxygen.templates,v 1.3 2010/07/06 09:20:12 mehner Exp $
#    Compiler  python 2.6.6
#     Company  Politecnico di Milano
#   Copyright  Copyright (c) 2011, Patrick Bellasi
#
# This source code is released for free distribution under the terms of the
# GNU General Public License as published by the Free Software Foundation.
# ============================================================================

import getopt
import os
import string
import sys

from pyx import *

metrics = ( \
"bq.sp.syncp.avg.time", \
"bq.sp.syncp.avg.pre",  \
"bq.sp.syncp.avg.sync", \
"bq.sp.syncp.avg.do",   \
"bq.sp.syncp.avg.post", \
"bq.sm.time",           \
"bq.sm.yamca.map",      \
"bq.sm.yamca.entities", \
"bq.sm.yamca.ord",      \
"bq.sm.yamca.mcomp",    \
"bq.sm.yamca.sel"       \
)

"""
m - Metrics to graph
c - List of clusters defining dataset
p - The fixed amout of PEs to consider
"""
def plotXexcYmetZcls(m, c, p):

    g = graph.graphxy(width=320*unit.w_pt,
            key=graph.key.key(pos="br", dist=0.1))
            #x=graph.axis.linear(min=0, max=5),

    plots = []
    for Ci in c:
        dataset = "graph_%s-w002c020-C%03dPE%03d.dat" % (m,Ci,p)
        print "Adding dataset: %s ..." % (dataset)
        dataset_key = "%02d Clusters" % (Ci)
        plots.append(graph.data.file(dataset, x="EXCs", y="Avg", dy="StdDev",
            title=dataset_key))
    
    g.plot(plots,
            [graph.style.line([color.gradient.Rainbow]),
                graph.style.errorbar()])

    graph_name = "graph_%s-w002c020-PE%03d.pdf" % (m,p)
    print "Plot graph %s ..." % (graph_name)
    g.writePDFfile(graph_name)
    g.pipeGS(filename=string.replace(graph_name, "pdf", "png"))

"""
m - Metrics to graph
c - The fixed amout of Clusters to consider
p - List of PEs to defining dataset
"""
def plotXexcYmetZpes(m, c, p):

    g = graph.graphxy(width=320*unit.w_pt,
            key=graph.key.key(pos="br", dist=0.1))
            #x=graph.axis.linear(min=0, max=5),

    plots = []
    for Pi in p:
        dataset = "graph_%s-w002c020-C%03dPE%03d.dat" % (m,c,Pi)
        print "Adding dataset: %s ..." % (dataset)
        dataset_key = "%02d PEs" % (Pi)
        plots.append(graph.data.file(dataset, x="EXCs", y="Avg", dy="StdDev",
            title=dataset_key))


    g.plot(plots,
            [graph.style.line([color.gradient.Rainbow]),
                graph.style.errorbar()])
        
    graph_name = "graph_%s-w002c020-C%03d.pdf" % (m,c)
    print "Plot graph %s ..." % (graph_name)
    g.writePDFfile(graph_name)
    string.replace(graph_name, "pdf", "png")
    g.pipeGS(filename=string.replace(graph_name, "pdf", "png"))

def getMetricsByStats(resluts_dir):
    flist = os.listdir(results_dir)
    for f in flist:
        pass
        

class Usage(Exception):
    def __init__(self, msg):
        self.msg = msg

def main(argv=None):
    if argv is None:
        argv = sys.argv

    # parse command line options
    try:
        try:
            opts, args = getopt.getopt(argv[1:], "h", ["help"])
        except getopt.error, msg:
             raise Usage(msg)

        # process options
        for o, a in opts:
            if o in ("-h", "--help"):
                print __doc__
                return 0
            
        # process arguments
        #for arg in args:
        #    process(arg) # process() is defined elsewhere

    except Usage, err:
        print >>sys.stderr, err.msg
        print >>sys.stderr, "for help use --help"
        return 2

    clusters = [1, 2, 4, 8, 16]
    pes = [4, 8, 16, 32]
    for m in metrics:
        _m = string.replace(m, ".", "_")
        for c in clusters:
            plotXexcYmetZpes(_m, c, pes)
        for p in pes:
            plotXexcYmetZcls(_m, clusters, p)

if __name__ == "__main__":
	sys.exit(main())

