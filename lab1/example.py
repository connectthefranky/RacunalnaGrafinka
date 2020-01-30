import pygame
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *

import pywavefront
from pywavefront import visualization

import numpy as np
from numpy import linalg as LA

display = (800,600)

B = np.array([[-1, 3, -3, 1], [3, -6, 3, 0], [-3, 0, 3, 0], [1, 4, 1, 0]])
dB = np.array([[-1, 3, -3, 1], [2, -4, 2, 0], [-1, 0, 1, 0]])
ddB = np.array([[-1, 3, -3, 1], [1, -2, 1, 0]])
R = np.empty(0)

def ucitajObjekt(name = "kocka.obj"):
    v = []
    f = []
    file = open(name, "r")
    line = file.readline()
    while (line):
        strippedLine = line.rstrip('\n').split(' ')
        if (strippedLine[0] == "v"):
            v.append(np.array([float(x) for x in strippedLine[1:]]))
        if (strippedLine[0] == "f"):
            f.append(np.array([int(x) for x in strippedLine[1:]]))
        line = file.readline()
    return np.array([np.array(v), np.array(f)])
    
def iscrtajObjekt(obj):
    glBegin(GL_TRIANGLES)
    for f in obj[1]:
        a = obj[0][f[0]-1]
        b = obj[0][f[1]-1]
        c = obj[0][f[2]-1]
        glVertex3f(a[0], a[1], a[2])
        glVertex3f(b[0], b[1], b[2])
        glVertex3f(c[0], c[1], c[2])
    glEnd()
    pass

def ucitajTockeAproksimacijeIzDatoteke():
    points = []
    file = open("tockeAproksimacije.txt", "r")
    line = file.readline()
    while (line):
        points.append([int(x) for x in line.rstrip('\n').split(' ')])
        line = file.readline()
    return np.array(points)

def odrediRotaciju(pocetnaOrijentacija, ciljnaOrijentacija):
    s = pocetnaOrijentacija
    e = ciljnaOrijentacija
    print(s, e)
    sNorm = LA.norm(s)
    eNorm = LA.norm(e)

    if (sNorm * eNorm == 0):
        return np.array([0, 0, 0, 0])
    else:
        osRotacije = odrediVektorskiProdukt(s, e)
        kutRotacije = np.arccos((s.dot(e))/(sNorm*eNorm))
        result = np.append(kutRotacije, osRotacije)
        return result

def odrediTranslaciju(t, segment):
    global B
    global R

    T = np.array([t**3, t**2, t, 1])
    return T.dot(1/6).dot(B).dot(R[segment-1:segment+3])

def odrediTangentu(t, segment):
    global dB
    global R

    T = np.array([t**2, t, 1])
    return T.dot(0.5).dot(dB).dot(np.array(R[segment-1:segment+3]))
    
def odrediDruguDerivacijuSegmenta(t, segment):
    T = np.array([t, 1])
    return T.dot(ddB).dot(R[segment-1:segment+3])
    
def odrediDCM(t, segment):
    tangenta = odrediTangentu(t, segment)
    drugaDerivacija = odrediDruguDerivacijuSegmenta(t, segment)
    normala = odrediVektorskiProdukt(tangenta, drugaDerivacija)
    binormala = odrediVektorskiProdukt(tangenta, normala)
    return np.vstack((np.vstack((tangenta, normala)), binormala)).T

def odrediVektorskiProdukt(a, b):
    return np.array([a[1]*b[2]-b[1]*a[2], b[0]*a[2]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]])

def odrediTransformiraneKoordinateObjekta(t, segment, rotation, translation, obj):
    glTranslate(-5, -5, -50)
    glTranslatef(translation[0], translation[1], translation[2])

    R = odrediDCM(t, segment)
    
    invR = LA.inv(R)
    obj[0] = obj[0].dot(invR)
    glRotatef(rotation[0]/np.pi*180, rotation[1], rotation[2], rotation[3])
    obj[0] = obj[0].dot(R)

    return obj
    
def iscrtajKrivulju(numberOfSegments):
    glTranslate(-5, -5, -50)
    for segment in range(1, numberOfSegments-1):
        s = odrediTranslaciju(0, segment)
        tangent = odrediTangentu(0, segment)
        glBegin(GL_LINES)
        glVertex3f(s[0], s[1], s[2])
        glVertex3f(s[0]+tangent[0], s[1]+tangent[1], s[2]+tangent[2])
        glEnd()
        glBegin(GL_LINE_STRIP)
        for t in range(51):
            t /= 50
            point = odrediTranslaciju(t, segment)
            glVertex3f(point[0], point[1], point[2])
        glEnd()
    pass

def resetViewMatrix():
    global display

    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()
    gluPerspective(45, (display[0]/display[1]), 0.1, 50)
    pass  

def main():
    global display
    global R

    obj = ucitajObjekt()
    R = ucitajTockeAproksimacijeIzDatoteke()

    numberOfSegments = len(R)-3
    pocetnaOrijentacija = R[0] + np.array([0, 1, 0]) 

    pygame.init()
    pygame.display.set_mode(display, DOUBLEBUF|OPENGL)

    while True:
        for segment in range(1, numberOfSegments-1):
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    pygame.quit()
                    quit()
            for t in range(51):
                t /= 50
                
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
                
                ciljnaOrijentacija = odrediTangentu(t, segment)
                rotation = odrediRotaciju(pocetnaOrijentacija, ciljnaOrijentacija)
                
                print(rotation)
                translation = odrediTranslaciju(t, segment)
                
                obj = odrediTransformiraneKoordinateObjekta(t, segment, rotation, translation, obj)
                iscrtajObjekt(obj)
                resetViewMatrix()
                
                iscrtajKrivulju(numberOfSegments)
                resetViewMatrix()

                pygame.display.flip()
                pygame.time.wait(1)

main()



