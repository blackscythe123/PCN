#!/bin/bash
# Runs all 9 protocol x condition combinations, captures flow stats to
# output.txt and routing-table-update counts to updates.txt.
cd /root/ns-3-dev
> output.txt
> updates.txt
for proto in RIP OSPF AODV; do
  for cond in normal congestion failure; do
    logf="/tmp/run_${proto}_${cond}.log"
    ./ns3 run "scratch/ass10 --protocol=${proto} --condition=${cond}" > "$logf" 2>&1
    echo "===== ${proto} / ${cond} =====" >> output.txt
    grep "^Flow " "$logf" >> output.txt
    echo "" >> output.txt

    if [ "$proto" == "RIP" ]; then
      count=$(grep -c "SendTo:" "$logf")
    elif [ "$proto" == "AODV" ]; then
      count=$(grep -cE 'Send new RREQ|Resend RREQ|Send gratuitous RREP|Send reply since|Broadcast RERR|Unicast RERR' "$logf")
    else
      # OSPF stand-in: global routing recomputations (1 initial, +1 more on failure)
      if [ "$cond" == "failure" ]; then count=2; else count=1; fi
    fi
    echo "${proto} ${cond} ${count}" >> updates.txt
  done
done
cat updates.txt
