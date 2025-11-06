# Configuración de las Islas del Lab

Se procede a realizar la configuración completa para la comunicación entre
las islas del laboratorio. Se asigna una VLAN a cada switch L2 para separar los
dominios y facilitar la asignación de direcciones. El L3 funciona como
router, ya que rutea los paquetes entre los dominios separados por las VLAN.

## Topología

Se utiliza la topología provista en el packet tracer, la cual
incluye las 7 islas, contando la de profesores. Como sea, así como
el profesor Ariel indicó en clase, solo se configuran las islas que
en el aula son empleadas por los 4 equipos de trabajo, por lo que
se procede a solo configurar las islas de los profes, y de la 2 a la 5, que
son las se usan a diario en el laboratorio físico.

## Configuración Básica

Configuración en L2 de Isla Profes:
```
enable
configure terminal
hostname IslaProfes
```

Configuración en L2 de Isla 2:
```
enable
configure terminal
hostname Isla2
```

Configuración en L2 de Isla 3:
```
enable
configure terminal
hostname Isla3
```

Configuración en L2 de Isla 3:
```
enable
configure terminal
hostname Isla3
```

Configuración en L2 de Isla 4:
```
enable
configure terminal
hostname Isla4
```

Configuración en L2 de Isla 5:
```
enable
configure terminal
hostname Isla5
```

Configuración en L3:
```
enable
configure terminal
hostname L3
```


## Configuración de VLAN

Configuración en L2 de Isla Profes:
```
vlan 600
name IslaProfes
```

Configuración en L2 de Isla 2:
```
vlan 620
name Isla2
```

Configuración en L2 de Isla 3:
```
vlan 630
name Isla3
```
Configuración en L2 de Isla 4:
```
vlan 640
name Isla4
```

Configuración en L2 de Isla 5:
```
vlan 650
name Isla5
```

Configuración en L3:
```
vlan 600
name IslaProfes
vlan 620
name Isla2
vlan 630
name Isla3
vlan 640
name Isla4
vlan 650
name Isla5
```

## Interfaces

Configuración L2 de Isla Profes:
```
! Asignarle IP a la VLAN
interface vlan 600
ip address 172.16.123.2 255.255.255.240
! Puerto trunk hacia L3
interface gigabitethernet 0/1
switchport mode trunk
switchport trunk allowed vlan 600
```

Configuración L2 de Isla 2:
```
! Asignarle IP a la VLAN
interface vlan 620
ip address 172.16.123.34 255.255.255.240
! Puerto trunk hacia L3
interface gigabitethernet 0/1
switchport mode trunk
switchport trunk allowed vlan 620
```

Configuración L2 de Isla 3:
```
! Asignarle IP a la VLAN
interface vlan 630
ip address 172.16.123.50 255.255.255.240
! Puerto trunk hacia L3
interface gigabitethernet 0/1
switchport mode trunk
switchport trunk allowed vlan 630
```

Configuración L2 de Isla 4:
```
! Asignarle IP a la VLAN
interface vlan 640
ip address 172.16.123.66 255.255.255.240
! Puerto trunk hacia L3
interface gigabitethernet 0/1
switchport mode trunk
switchport trunk allowed vlan 640
```

Configuración L2 de Isla 5:
```
! Asignarle IP a la VLAN
interface vlan 650
ip address 172.16.123.82 255.255.255.240
! Puerto trunk hacia L3
interface gigabitethernet 0/1
switchport mode trunk
switchport trunk allowed vlan 650
```

Configuración L3:
```
ip routing
! Asignarle IP a la VLAN
interface vlan 600
ip address 172.16.123.1 255.255.255.240
interface vlan 620
ip address 172.16.123.33 255.255.255.240
interface vlan 630
ip address 172.16.123.49 255.255.255.240
interface vlan 640
ip address 172.16.123.65 255.255.255.240
interface vlan 650
ip address 172.16.123.81 255.255.255.240
! Puerto trunk hacia L2
interface fa0/1
switchport trunk encapsulation dot1q
switchport mode trunk
switchport trunk allowed vlan 600
interface fa0/2
switchport trunk encapsulation dot1q
switchport mode trunk
switchport trunk allowed vlan 620
interface fa0/3
switchport trunk encapsulation dot1q
switchport mode trunk
switchport trunk allowed vlan 630
interface fa0/4
switchport trunk encapsulation dot1q
switchport mode trunk
switchport trunk allowed vlan 640
interface fa0/5
switchport trunk encapsulation dot1q
switchport mode trunk
switchport trunk allowed vlan 650
```

## DHCP

Configuración L2 de Isla Profes:
```
interface range fastethernet 0/1-4
switchport mode access
switchport access vlan 600
spanning-tree portfast

service dhcp
ip dhcp pool VLAN600
network 172.16.123.0 255.255.255.240
default-router 172.16.123.1
dns-server 8.8.8.8
ip dhcp excluded-address 172.16.123.1
ip dhcp excluded-address 172.16.123.2
```

Configuración L2 de Isla 2:
```
interface range fastethernet 0/1-4
switchport mode access
switchport access vlan 620
spanning-tree portfast

service dhcp
ip dhcp pool VLAN620
network 172.16.123.32 255.255.255.240
default-router 172.16.123.33
dns-server 8.8.8.8
ip dhcp excluded-address 172.16.123.33
ip dhcp excluded-address 172.16.123.34
```

Configuración L2 de Isla 3:
```
interface range fastethernet 0/1-4
switchport mode access
switchport access vlan 630
spanning-tree portfast

service dhcp
ip dhcp pool VLAN630
network 172.16.123.48 255.255.255.240
default-router 172.16.123.49
dns-server 8.8.8.8
ip dhcp excluded-address 172.16.123.49
ip dhcp excluded-address 172.16.123.50
```

Configuración L2 de Isla 4:
```
interface range fastethernet 0/1-4
switchport mode access
switchport access vlan 640
spanning-tree portfast

service dhcp
ip dhcp pool VLAN640
network 172.16.123.64 255.255.255.240
default-router 172.16.123.65
dns-server 8.8.8.8
ip dhcp excluded-address 172.16.123.65
ip dhcp excluded-address 172.16.123.66
```

Configuración L2 de Isla 5:
```
interface range fastethernet 0/1-4
switchport mode access
switchport access vlan 650
spanning-tree portfast

service dhcp
ip dhcp pool VLAN650
network 172.16.123.80 255.255.255.240
default-router 172.16.123.81
dns-server 8.8.8.8
ip dhcp excluded-address 172.16.123.81
ip dhcp excluded-address 172.16.123.82
```

## Testing

Para hacer las pruebas, como hay una sola isla, se debe realizar ping entre las
computadoras.