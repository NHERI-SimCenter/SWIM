

model BasicBuilder -ndm 2 -ndf 2

#These are the problem variables
pset beta 5.462261077231849e-01   
pset Ap 9.665784174420770e-02 
pset An 3.528346194898871e+00 
pset Bn 4.914497665322617e-01

# define nodes: node nodeTag x y
node 1 0 0 
node 2 7.5785 0 
node 3 7.5785 11.3329 
node 4 0 11.3329 
node 5 7.5785 22.6657 
node 6 0 22.6657 
node 7 7.5785 33.9986 
node 8 0 33.9986 


# define uniaxialMaterial for rebars, uniaxialMaterial matType? $Fy $E0 $b
uniaxialMaterial Steel01 1 84.6555 29000 0.01 
uniaxialMaterial Steel01 2 83.5388 29000 0.01 
uniaxialMaterial Steel01 3 75.2574 29000 0.01 

# nDMaterial PlaneStressRebarMaterial matTag matTag_for_uniaxialMaterial angle_of_rebar 
nDMaterial PlaneStressRebarMaterial 1 1 90 
nDMaterial PlaneStressRebarMaterial 2 2 90 
nDMaterial PlaneStressRebarMaterial 3 3 0 


# define pars for PlasticDamageConcretePlaneStress
set E 4635.43;# ksi 
set nu 0.2 
set fc [expr 6.61349*0.7] ;# ksi 
set ft [expr $fc*0.25] ;# ksi 

#source parameters.tcl
#set beta 0.5
#set Ap 0.5
#set An 0.5
#set Bn 0.5
set outPutFileName outputs.out 
nDMaterial PlasticDamageConcretePlaneStress 4 $E $nu $ft $fc $beta $Ap $An $Bn ;# concrete 

# nDMaterial PlaneStressLayeredMaterial matTag num_of_layers layer_1_Tag  layer_1_thickness 1 layer_2_Tag  layer_2_thickness layer_3_Tag  layer_3_thickness
nDMaterial PlaneStressLayeredMaterial 5 3 4 5.85852 1 0.0316793 3 0.0148032  
nDMaterial PlaneStressLayeredMaterial 6 3 4 5.81253 2 0.0693821 3 0.0230882  
nDMaterial PlaneStressLayeredMaterial 7 3 4 5.85852 1 0.0316793 3 0.0148032  
nDMaterial PlaneStressLayeredMaterial 8 3 4 5.81253 2 0.0693821 3 0.0230882  

# define elements: element quad eleTag i j k l thickness PlaneStress matTag
element quad 0 1 2 3 4 1 PlaneStress 6
element quad 1 4 3 5 6 1 PlaneStress 6
element quad 2 6 5 7 8 1 PlaneStress 6


# define time series
timeSeries Constant 1 -factor 0
timeSeries Constant 2 -factor 1
timeSeries Constant 3 -factor 0
timeSeries Path 4 -dt 1 -values { 0 0.0873127 0.134938 0.1905 0.254001 0.317501 0.412751 0.381001 0.309563 0.198438 0.111125 0.0793752 0.00793752 -0.0238125   }
set nPts 14

 # Fix The Bottom Nodes
 # pattern Plain patternTag timeSeriesTag 
pattern Plain 1 1 {
  # sp nodeTag dof value
  sp 1 1 0
  sp 1 2 0
  sp 2 1 0
  sp 2 2 0
}

 # Vertical Loads on Top, do not include weight 
pattern Plain 2 2 {
  # load nodeTag force_at_dof_1 force_at_dof_2
  load 7 0.0 -36.8672
  load 8 0.0 -36.8672
}

 # Some Horizontal Load on top
pattern Plain 3 3 {
  load 7 0.5 0.0
  load 7 0.0 0.244983
  load 8 0.5 0.0
  load 8 0.0 -0.244983
}

 # DisplacementControlAtTop 
pattern Plain 4 4 {
  sp 7 1 1
  sp 8 1 1
}

# add self weight

set numNodes 6 
set rho 8.60914e-05 
set g -1.0
set volumeOfWall 73.7705

pattern Plain 100 "Constant" { 
load 7 0.0 [expr ($volumeOfWall*$rho*$g)/$numNodes];#SELF-WEIGHT
load 8 0.0 [expr ($volumeOfWall*$rho*$g)/$numNodes];#SELF-WEIGHT
} 

# specify the solver
constraints Plain
system UmfPack
test NormDispIncr 1.0e-3 30
algorithm ModifiedNewton
numberer RCM
integrator LoadControl 1
analysis Static
analyze 1

puts "gravity is done."


lappend EDPNodes 1
lappend EDPNodes 2
eval "recorder Node -file $outPutFileName -node $EDPNodes -dof 1 reaction"
eval "recorder Node -file disp.out -nodeRange 1 8 -dof 1 2 disp"

system UmfPack 
constraints Penalty 1.0e10 1.0e10 
test NormDispIncr 1.0e-4 50 0 
algorithm Newton 
numberer RCM 
integrator LoadControl 1. 
analysis Static 
for {set i 0} {$i < [expr $nPts]} {incr i 1} { 
set ok [analyze 1] 
if {$ok != 0} { 
test NormDispIncr 1.0e-3 1000 0 
algorithm Newton -initial 
set ok [analyze 1] 
test NormDispIncr 1.0e-4 500 0 
algorithm Newton 
} 
if {$ok != 0} {  
set i [expr $nPts]; 
} 
} 
puts "cyclic is done."

remove recorders 
wipe  
