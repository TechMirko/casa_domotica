Board, tecnologie e linguaggi
ESP32 (C++)
Node-red (NodeJS)
MQTT Protocol
I2C Protocol

Installazione e set-up
Per prima cosa procediamo ad installare Node-red, ma ancora prima dobbiamo installare NodeJS che altro non è che una dipendenza di JavaScript che ci permette di creare degli applicativi molto efficienti per quanto riguarda le comunicazioni web. Fatto ciò possiamo procedere ad installare Node-red che ci permetterà di creare le dashboard. Una volta avviato Node-red possiamo accedere all’indirizzo localhost:1880 che ci porta all’editor di Node-red da cui dovremo installare i pacchetti chiamati “dashboard”. Una volta creata la nostra dashboard possiamo premere il pulsantone “DEPLOY” e accedere a localhost:1880/ui dove vedremo l’effettiva dashboard appena creata (per velocizzare i tempi sotto verrà riportato il codice JSON della dashboard). 
Ora possiamo avviare Arduino IDE in qualunque versione preferiate, installate le librerie presenti nella cartella "Dependencies" copiando ed incollando i folder all’interno del folder al path “../Documenti/Arduino/Librerie”. Successivamente incolliamo il codice sotto riportato, selezioniamo la port adatta e carichiamo il software sul nostro ESP32, MOLTO IMPORTANTE leggere le prime righe di codice nelle quali sono presenti dei commenti che facilitano il corretto set-up. 
Se tutto è stato collegato correttamente, avremo in funzione la nostra centralina di casa domotica.
