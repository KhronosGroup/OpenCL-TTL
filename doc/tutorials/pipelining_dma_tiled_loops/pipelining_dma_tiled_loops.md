# Pipelining DMA Tiled Loops

## CONTENTS: <!-- omit in toc -->

- [Pipelining DMA Tiled Loops](#pipelining-dma-tiled-loops)
  - [Introduction](#introduction)
  - [Maximizing performance](#maximizing-performance)
  - [Appendix: full pipeline description](#appendix-full-pipeline-description)

## Introduction

As a simple example suppose a single input image is imported, operated on, and
exported. Tiling produces a single “outer” loop which conceptually processes one
tile per iteration:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // 0. Non-pipelined tiled loop
 for tile in [0, number_of_tiles]
   Import tile
   WaitImport tile
   Compute tile
   Export tile
   WaitExport tile
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Software-Pipelining a tiled loop can overlap independent operations across
iterations and execute them concurrently, similar to pipelining the instructions
of innermost loop(s) producing parallel instruction bundles for VLIW targets
Modulo-Scheduling is an automatic pipelining technique which
maximizes parallelism and throughput by minimizing the latency of the steady
state, aka Initiation Interval (II), and then applying variable expansion as
needed - allocating multiple rotating registers to values whose live-range
exceeds II. The concurrency in DMA-Tiled loops stems from the asynchronous and
independent nature of DMA operations, rather than the Instruction-Level
Parallelism of VLIWs. On many accelerators, a DMA import and/or export can run
in parallel to independent non-DMA SSC operations. Buffers allocated on scratchpad
memory whose pipelined live-ranges self-intersect, i.e., exceed the initiation
interval, require Array Expansion (across distinct memory banks), similar to
allocating multiple rotating registers.

It is also conceivable to combine manual Tiling with automated Pipelining, coupled
with manual or automated expansion. One such combination is initially outlined
below. Automated steps could be assisted by user directive, as in 
\#pragma disable_loop_pipelining of Intel FPGA SDK for OpenCL.

## Maximizing performance

The above chain of 5 operations: Import --\> WaitImport --\> Compute --\> Export
--\> WaitExport can be maximally pipeline-parallelized by assigning
Import, Export and Compute to three distinct pipeline stages: starting to import
the next tile in parallel to computing the current tile, and in parallel to
exporting the previously computed tile

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // Conceptual 3-stage Pipeline 
 // II = max {|Import|, |Export|, |Compute|}

 Iteration:          1        2        3+
 ---------------+--------+--------+--------
 Prolog Stage 1: Import1
                --------
 Prolog Stage 2: Compute1 Import2
                 -------- --------
 Steady State:   Export1  Compute2 Import3
                 -------- -------- --------
 Epilog Stage 1:          Export2  Compute3
                          -------- --------
 Epilog Stage 2:                   Export3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The buffer associated with Import has a live-range spanning two iterations:
starting at the Import operation and ending when the associated Compute finishes
to consume the buffer. The buffer associated with Export also has a live-range
spanning two iterations: starting at the associated Compute operation and ending
when Export finishes. Therefore, each requires double-buffering.

Q: When should WaitImport and WaitExport be scheduled? Their latency is expected
to be negligible compared to that of the actual Import and Export.

A: In general, on non-VLIW targets such as SSC, operations scheduled to run
together in parallel bundles need to be ordered in order to achieve the desired
concurrency. The parallel bundle in the above example:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 [Export-previous-tile || Computing-current-tile || Import-next-tile]
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

whose operations conceptually overlap by starting together and finishing
together, should be ordered such that Import and Export are issued before
Compute, which in turn is issued before WaitImport and WaitExport. Doing so
produces

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // 1a. “Earliest Waits” double double-buffered pipeline
 for (tile)
   Export tile-1; Import tile+1; Compute tile; WaitImport tile+1; WaitExport tile-1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  
Q0: Should WaitImport and/or WaitExport tile-1 be scheduled later?

A0: Pipelining optimizes the schedule according to latency estimates, so
synchronizing at the end of each “line” is expected to be quick. But it may be
advantageous to tolerate potential delays in DMA transactions by postponing
their synchronizing waits, provided resources are available to do so - including
associated scratchpad buffer (on available memory bank) and DMA channel.

Rotating WaitImport tile+1 and WaitExport tile-1 forward from being the last
operations of the iteration, to being the first operations of the next
iteration, produces the equivalent pipeline with one additional stage and
iteration:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // 1b. Rotated “Earliest Waits” double double-buffered pipeline
 for (tile)
   WaitImport tile; WaitExport tile-2; Export tile-1; Import tile+1; Compute tile 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Postponing WaitImport tile to appear after WaitExport, Export, and right before
Import tile+1 produces

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // 2. “Wait before next Import/Export” pipeline
 for (tile)
   WaitExport tile-2; Export tile-1; WaitImport tile; Import tile+1; Compute tile 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is the pipeline produced by dmaiterate, and should be used when analyzing
performance comparison between dmaiterate and manual/TTL tiling (see Appendix A
in “To Tiler Or Not To Tiler”).

Postponing WaitImport and WaitExport further/furthest, to appear before the
dependent Compute produces

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // 3. “Latest Waits” pipeline
 for (tile)
   Export tile-1; Import tile+1; WaitImport tile; WaitExport tile-2; Compute tile
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A full pipeline description is provided in an Appendix below.

Q1: If WaitImport tile and Import tile+1, are scheduled back-to-back, who should
go first?

A1: placing WaitImport tile first allows both to reuse the same channel. If
WaitImport tile stalls - because the import of tile is yet to complete, issuing
Import tile+1 early will not gain much (if anything?) given that all imports are
DMA’d in-order on any device. Same goes for scheduling WaitExport tile-2 relative
to Export tile-1.

Q2: Should WaitImport tile be scheduled after Export tile-1, or before it?

A2: When hardware allows it may be advantageous to schedule Export tile-1 first
before WaitImport tile which may stall, if there are available channels. Same
goes for scheduling WaitExport tile-2 relative to Import tile+1.

Q3: Should WaitImport tile and WaitExport tile-2 be combined into a single wait
on both channels, instead of two separate waits scheduled back to back?

A3: probably yes, resulting in a single DMA/SSC synchronization point instead of
two(?)

Q3.5: How should the above example be pipelined, if fewer than four
buffers/banks are available?

A3.5: With fewer than four buffers, at most two of {Compute, Import, Export} can
overlap, implying that II will be at-least “2”. At-least two distinct buffers
are required by Compute, assuming it cannot write destructively to the same
buffer it reads from.

Overlapping Import with Export is possible and requires (only the) two buffers:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // Conceptual 2-stage Pipeline with Duplex DMA
 // W/o any double-buffering
 // II = |Compute| + max {|Import|, |Export|}

 Iteration:          1        2+
 ---------------+--------+--------+
 Prolog Stage 1: Import1
                 Compute1
                --------
 Steady State:   Export1  Import2
                          Compute2
                 -------- --------
 Epilog Stage 1:          Export2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If Import is not overlapped with Export, either or both could be overlapped with
Compute, requiring a third buffer for double or triple buffering, respectively.
Doing so also addresses the following related question:

Q4: How should the above example be pipelined when duplex DMA is not possible

A4: There are several pipelining options, depending on the latency of Compute
relative to that of Import and Export

A4.1: Overlap Compute with Import; good when \|Compute\| \<= \|Import\|

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // Conceptual 2-stage Pipeline with Simplex DMA
 // II = |Export| + max {|Compute|, |Import|}
 // Import is double-buffered but Export is not

 Iteration:          1        2+
 ---------------+--------+--------+
 Prolog Stage 1: Import1

 Steady State:   Compute1 Import2
                 Export1
 Epilog Stage 1:          Compute2
                          Export2  
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A4.2: Overlap Compute with Export; good when \|Compute\| \<= \|Export\|

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // Conceptual 2-stage Pipeline with Simplex DMA
 // II = |Import| + max {|Compute|, |Export|}
 // Export is double-buffered but Import is not

 Iteration:          1        2+
 ---------------+--------+--------+
 Prolog Stage 1: Import1
                 Compute1
 Steady State:            Import2
                 Export1  Compute2
 Epilog Stage 1:          
                          Export2  
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Q5: If \|Compute\| \<= min {\|Input\|, \|Export\|}, which of A4.1 and A4.2 is
better?

A5: their II’s in this case are equal: \|Input\|+\|Export\|; double-buffering
the smaller buffer (of Import and Export) would save scratchpad memory space.

A4.3: Overlap Compute with both Import and Export; good also when \|Compute\| \>
max {\|Import\|, \|Export\|}

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // Conceptual 3-stage Pipeline with Simplex DMA
 // II = max {|Compute|, |Import|+|Export|}
 // Import and Export are triple-buffered together

 Iteration:          1        2+
 ---------------+--------+--------+
 Prolog Stage 1:
                 Import1                   
 Prolog Stage 2: Compute1                  -+ Live-range of
                 (cont’d) Import2           | exported buffer
 Steady State:   Export1  Compute2         -+ 
                          (cont’d) Import3  -+ Live-range of
 Epilog Stage 1:          Export2  Compute3  | imported buffer
                                   (cont’d) -+
 Epilog Stage 2:                   Export3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note: A4.3 can be applied instead of A4.1 and A4.2 when \|Compute\| \<=
\|Import\| and \|Compute\| \<= \|Export\|, respectively: they have the same II
of \|Input\|+\|Export\|. The triple-buffering of A4.3 requires three buffers all
having the size of the larger buffer (between Import and Export), whereas A4.1
and A4.2 require smaller scratchpad memory space if Import and Export buffers
are of distinct sizes.

Note: triple-buffering advocates the use of duplex DMA which may reduce the
II from max {\|Compute\|, \|Input\|+\|Export\|} potentially close to max
{\|Compute\|,\|Input\|,\|Export\|}.

Note: if Compute can be partitioned, the loop can be rotated to produce a
2-staged pipeline

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 // Conceptual 2-stage Rotated Pipeline with Simplex DMA
 // II = max {|Compute|, |Import|+|Export|}
 // Import and Export are triple-buffered together

 Iteration:          1        2+
 ---------------+--------+--------+
 Prolog Stage 1: Import1
                 CompuA1                   
 Steady State:   CompuB1  Import2
                 Export1  CompuA2
 Epilog Stage 1:          CompuB2 
                          Export2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

## Appendix: full pipeline description

Including WaitImport tile and WaitExport in the pipeline, having a single column
for single-issue targets such as SSC, produces the following for the above
example, using an estimate of \~100 cycles for each DMA transaction and
computation:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // 3a. “Latest Waits” pipeline: full description

  0              -+
  1    Import0    |
  2               | Prolog Stage 1
  3               |
  4              -+
100                -+
101       Import1   |
102    WaitImport0  | Prolog Stage 2
103                 |
104    Compute0    -+
200    Export0        -+
201          Import2   |
202       WaitImport1  | Prolog Stage 3
203                    |
204       Compute1    -+
300       Export1        -+
301             Import3   |
302          WaitImport2  | Steady State
303    WaitExport0        |
304          Compute2    -+
400          Export2        -+
401                          |
402             WaitImport3  | Epilog Stage 1
403       WaitExport1        |
404             Compute3    -+
500             Export3     -+
501                          |
502                          | Epilog Stage 2
503          WaitExport2     |
504                         -+
600                         -+
601                          |
602                          | Epilog Stage 3
603             WaitExport3  |
604                         -+
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
