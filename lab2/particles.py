import pygame, OpenGL, math, random
from pygame.locals import *
from OpenGL.GL import *
from OpenGL.GLU import *
from time import time

gravity = 80

class Particle:
    def __init__(self, x, y, ttl, color):
        self.x = x
        self.y = y
        self.z = 0
        self.ttl = ttl
        self.speedX = random.randint(-85, 85)
        self.speedY = random.randint(250, 300)
        self.timeAlive = 0
        self.startTime = time() + random.randint(0, 400)/100
        self.color = color
    
    def getPosition(self):
        global gravity
        x = self.x + self.speedX * self.timeAlive
        y = self.y - self.speedY * self.timeAlive + gravity * self.timeAlive * self.timeAlive
        return x, y
        
    def update(self, t):
        self.timeAlive = time() - self.startTime
        
    def isAlive(self):
        return self.timeAlive < self.ttl
        
class ParticleSystem:
    def __init__(self, x, y, z, n, ttl, color):
        self.x = x
        self.y = y
        self.z = z
        self.particles = [Particle(x, y, ttl, color) for _ in range(n)]
        
    def getParticles(self):
        return self.particles
        
def resetView():
    glLoadIdentity()
    gluPerspective(45, 1, 0.05, 100)
    glTranslatef(0,0,-5)
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT)

#----------------------------------------------------    
    
pygame.init()
screen = pygame.display.set_mode((600,600))

BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
RED = (255, 0, 0)
BLUE = (0, 0, 255)

surfaceSize = (50, 50)

#Circle particle
circle = pygame.Surface(surfaceSize)
circle.set_colorkey(BLACK)

#Particle texture
image = pygame.Surface(surfaceSize, pygame.SRCALPHA, 32)
image = image.convert_alpha()
texture = pygame.image.load("vulcano.png").convert_alpha()
texture = pygame.transform.scale(texture,(100, 100))
texture_rect = texture.get_rect()
texture_rect.center = (325, 450)
screen.blit(texture, texture_rect)

particleSize = 6
ttl = 3
numberOfParticles = 1000
sysPos = (300, 400, 0)

particleSystem = ParticleSystem(sysPos[0], sysPos[1], sysPos[2], numberOfParticles, ttl, WHITE)

while True:
    now = time()
    screen.fill(WHITE)
    screen.blit(texture,texture_rect)
    resetView()
    
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
    
    for i in range(numberOfParticles):
        particle = particleSystem.getParticles()[i]
        
        if (particle.startTime < now):
            particle.update(time())
            
            if (particle.isAlive()):
                x, y = particle.getPosition()
                
                color = particle.color
                if (color[2] > 2):
                    color = (color[0], color[1], color[2]-3)
                if (color[1] > 1):
                    color = (color[0], color[1]-2, color[2])
                if (color[0] > 0):
                    color = (color[0]-1, color[1], color[2])
                particle.color = color
                
                #screen.blit(texture, (x,y))
                pygame.draw.circle(circle, particle.color, (25, 25), particleSize)
                screen.blit(circle, (x, y))
            else:
                particleSystem.particles[i] = Particle(sysPos[0], sysPos[1], ttl, WHITE)

    pygame.display.flip()
    pygame.time.wait(10)
    
    
    
    
        
