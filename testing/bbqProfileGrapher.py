#!/usr/bin/python 
#
#       @file  bbqProfileGrapher.py
#      @brief  Barbeque RTRM Profiling and Graphinc application
#
# This is a simple script which produces a scheduling graph out of a Barbeque RTRM
# log file.
#
#     @author  Patrick Bellasi (derkling), derkling@gmail.com
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

metrics = [ \
("bq.ym.syncp.avg.time", "SyncP",                   "Time [ms]"), \
("bq.ym.syncp.avg.pre",  "PreChange",               "Time [ms]"), \
("bq.ym.syncp.avg.sync", "SyncChange",              "Time [ms]"), \
("bq.ym.syncp.avg.do",   "DoChange",                "Time [ms]"), \
("bq.ym.syncp.avg.post", "PostChange",              "Time [ms]"), \
("bq.sm.time",           "Scheduler",               "Time [ms]"), \
("bq.sm.yamca.map",      "Scheduler Data",          "Size [Bytes]"), \
("bq.sm.yamca.entities", "Entities",                "Count"), \
("bq.sm.yamca.ord",      "Scheduler Ordering",      "Time [ms]"), \
("bq.sm.yamca.mcomp",    "Scheduler Metrics",       "Time [ms]"), \
("bq.sm.yamca.sel"       "Scheduler Selection",     "Time [ms]") \
]

#recipes = ["1Awm1PEPrio1"]
recipes = ["BbqRTLibTestApp01AWM", "BbqRTLibTestApp03AWM", "BbqRTLibTestApp06AWM", "BbqRTLibTestApp10AWM"]
cores = [4]
clusters = [3]
pes = [4]


"""
m - Metrics to graph
r - The recipe used to produce the data
h - The fixed aount of Host Cores (HC) defining the dataset
c - List of clusters defining dataset
p - The fixed amout of PEs to consider
"""
def plotXexcYmetZcls(m, r, h, c, p):

    # The metrics to be plotted
    mk = string.replace(m[0], ".", "_")
    print "Plotting: %s (%s) vs (EXCs, Clusters)..." % (mk, m[1])

    # Setup graph geometry, axis and legend
    g = graph.graphxy(width=8, ratio=16./9,
            key=graph.key.key(pos="tl"),
            x=graph.axis.linear(title="EXCs Count"),
            y=graph.axis.linear(title=m[2]))

    # Add dataset to be plotted
    plots = []
    for Ci in c:
	dataset = "graph-%s-%s-HC%02d-w006c060-C%03dPE%03d.dat" % (mk,r,h,Ci,p)
        print "Adding dataset: %s ..." % (dataset)
        dataset_key = "%02d Clusters" % (Ci)
        plots.append(graph.data.file(dataset, x="EXCs", y="Avg", dy="StdDev",
            title=dataset_key))
    
    # Plot the dataset on the graph
    g.plot(plots,
            [graph.style.line([color.gradient.Rainbow,
                style.linestyle.solid]),
                graph.style.errorbar()])

    # Now plot the text, horizontally centered 
    graph_title = "%s vs EXCs (%s)" % (m[1], r) 
    g.text(g.width/2, g.height + 0.2, graph_title,
            [text.halign.center, text.valign.bottom, text.size.large])

    graph_name = "graph-%s-%s-HC%02d-w006c060-PE%03d.pdf" % (mk,r,h,p)
    print "Plot graph %s ..." % (graph_name)
    g.writePDFfile(graph_name)
    g.pipeGS(filename=string.replace(graph_name, "pdf", "png"))

"""
m - Metrics to graph
r - The recipe used to produce the data
h - The fixed amount of Host Cores (HC) defining the dataset
c - The fixed amount of Clusters to consider
p - List of PEs defining the dataset
"""
def plotXexcYmetZpes(m, r, h, c, p):

    # The metrics to be plotted
    mk = string.replace(m[0], ".", "_")
    print "Plotting: %s (%s) vs (EXCs, PEs)..." % (mk, m[1])

    # Setup graph geometry, axis and legend
    g = graph.graphxy(width=8, ratio=16./9,
            key=graph.key.key(pos="tl"),
            x=graph.axis.linear(title="EXCs Count"),
            y=graph.axis.linear(title=m[2]))

    # Add dataset to be plotted
    plots = []
    for Pi in p:
	dataset = "graph-%s-%s-HC%02d-w006c060-C%03dPE%03d.dat" % (mk,r,h,c,Pi)
        print "Adding dataset: %s ..." % (dataset)
        dataset_key = "%02d PEs" % (Pi)
        plots.append(graph.data.file(dataset, x="EXCs", y="Avg", dy="StdDev",
            title=dataset_key))

    # Plot the dataset on the graph
    g.plot(plots,
            [graph.style.line([color.gradient.Rainbow,
                style.linestyle.solid]),
                graph.style.errorbar()])
        
    # Now plot the text, horizontally centered 
    graph_title = "%s vs EXCs (%s)" % (m[1], r) 
    g.text(g.width/2, g.height + 0.2, graph_title,
            [text.halign.center, text.valign.bottom, text.size.large])

    graph_name = "graph-%s-%s-HC%02d-w006c060-C%03d.pdf" % (mk,r,h,c)
    print "Plot graph %s ..." % (graph_name)
    g.writePDFfile(graph_name)
    string.replace(graph_name, "pdf", "png")
    g.pipeGS(filename=string.replace(graph_name, "pdf", "png"))


"""
m - Metrics to graph
r - The recipe used to produce the data
h - List of Host Cores (HC) defining the dataset
c - The fixed amount of Clusters to consider
p - The fixed amount of PEs to consider
"""
def plotXexcYmetZhc(m, r, h, c, p):

    # The metrics to be plotted
    mk = string.replace(m[0], ".", "_")
    print "Plotting: %s (%s) vs (EXCs, HC)..." % (mk, m[1])

    # Setup graph geometry, axis and legend
    g = graph.graphxy(width=8, ratio=16./9,
            key=graph.key.key(pos="tl"),
            x=graph.axis.linear(title="EXCs Count"),
            y=graph.axis.linear(title=m[2]))

    # Add dataset to be plotted
    plots = []
    for Hi in h:
	dataset = "graph-%s-%s-HC%02d-w006c060-C%03dPE%03d.dat" % (mk,r,Hi,c,p)
        print "Adding dataset: %s ..." % (dataset)
        dataset_key = "%02d HCores" % (Hi)
        plots.append(graph.data.file(dataset, x="EXCs", y="Avg", dy="StdDev",
            title=dataset_key))
    
    # Plot the dataset on the graph
    g.plot(plots,
            [graph.style.line([color.gradient.Rainbow,
                style.linestyle.solid]),
                graph.style.errorbar()])
        
    # Now plot the text, horizontally centered 
    graph_title = "%s vs EXCs (%s)" % (m[1], r) 
    g.text(g.width/2, g.height + 0.2, graph_title,
            [text.halign.center, text.valign.bottom, text.size.large])

    graph_name = "graph-%s-%s-w006c060-C%03dPE%03d.pdf" % (mk,r,c,p)
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

    for Mi in metrics:
        for Ri in recipes:
        
            # Plotting Z=HCores
            for Ci in clusters:
                for Pi in pes:
                    plotXexcYmetZhc(Mi, Ri, cores, Ci, Pi)

            # Plotting Z=PEs
            for Hi in cores:
                for Ci in clusters:
                    plotXexcYmetZpes(Mi, Ri, Hi, Ci, pes)
                    
            # Plotting Z=Clusters
            for Hi in cores:
                for Pi in pes:
                    plotXexcYmetZcls(Mi, Ri, Hi, clusters, Pi)

if __name__ == "__main__":
	sys.exit(main())

