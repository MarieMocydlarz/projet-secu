
#!/bin/bash

bleuclair='\e[1;34m'
neutre='\e[0;m'
orange='\e[0;33m'
# Affichage des paquets installer
echo -ne "${orange}Les paquets qui sont installés sont:${bleuclair}\n"
dpkg-query -W | awk -F" " '{print $1}'

echo -ne "${orange}Les paquets à mettre à jour:${bleuclair}\n"
sudo apt-get --simulate upgrade | grep updates | awk -F" " '{print $2}' 
echo -ne "${neutre}"

