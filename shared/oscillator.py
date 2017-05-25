import math

pico = 1.0 / 1000000000000

Cl = 18 * pico
C0 = 7 * pico # Shunt capacitance
ESR = 80

gm = 25.0 / 1000 # 25mA/V

Cs = 5 * pico
F = 8000000

Cl1 = 2.0 * (Cl - Cs)
Cl2 = Cl1
Rext = 0 # 1 / (2.0 * math.pi * F * Cl2)

print "Rext: ", Rext

print "Cl: ", (Cl1 / pico)


gmcrit = 4 * (ESR + Rext) * (2 * math.pi * F) * (2 * math.pi * F) * (C0 + Cl) * (C0 + Cl)
print "Gain: ", (gm / gmcrit)

Vpp = 3.3 # as measured by oscilloscope ??? 1Volt
Cprobe = 1.6 * pico # Rigol probes capacitance
Ctot =  Cl1 + (Cs/2) + Cprobe
DL = ESR * (math.pi * F * Ctot) * (math.pi * F * Ctot) * Vpp * Vpp / 2.0
print "DL: ", (DL * 1000000) # micro watts
