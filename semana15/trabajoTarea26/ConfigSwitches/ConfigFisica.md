# Configuración de las Islas en el Laboratorio Físico

Se realiza la configuración física de un equipo Catalyst 2960 L2 y otro Catalyst
3560 L3.

Para ingresar a los switches se conecta directamente a la interfaz de consola y
con minicom se ingresa a la configuración. El L2 ocupa que en configuración se
coloque en Bps/pap/bits, 9600 8N1.


# Configuración del L3

Para el L3, solo hay que colocar las interfaces de las islas, las VLAN y activar
el routing. Lo diferente a la vez anterior es que esta vez se
incluyen todas las demás islas, aunque no estén conectadas.

```
! Poner nombre al L3
enable
configure terminal
hostname L3
! Nombrar VLAN
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
! Configurar las Interfaces
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
! Activar routing
ip routing
```

# Configuración del L2

Para el L2, solo se configura el de la isla de mi grupo. Esta sería
la isla 5 de acuerdo con la distribución física del laboratorio.
Se activa el servicio DHCP y se configuran las interfaces para
4 computadoras entrantes y el puerto del switch L3.

```
! Nombrar el L2
enable
configure terminal
hostname Isla5
! Nombrar VLAN
vlan 650
name Isla5
! Configurar las Interfaces
interface vlan 650
ip address 172.16.123.82 255.255.255.240
! Puerto trunk hacia L3
interface gigabitethernet 0/1
switchport mode trunk
switchport trunk allowed vlan 650
! Servicio DHCP
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