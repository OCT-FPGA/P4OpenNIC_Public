This pcap file contains eight ipv4 packets.

Send these packets through certain tools like tcpreplay. 

Before loading tables, you should see all eight packets in the network traffic. 

The driver contains four table entries with keys and masks. These entries will match exactly four packets in the pcap file. In other words, these four packets will be dropped. 

The driver will firstly insert these four entries and then check on them. Next it will demonstrate of updating them and deleting them. In the end, two of four entries has actually inserted. Thus two packets will be dropped. 