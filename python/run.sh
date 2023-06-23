read -r id
echo $id
cd ~/hadoop/
reportdir=~/hadoop/hadoop-hdfs-project/hadoop-hdfs/target/surefire-reports/
logfile=$reportdir/org.apache.hadoop.hdfs.server.blockmanagement.TestPendingReplication-output.txt
instrumentation_plan_dir=~/log/log_compare/python/plans
rm $reportdir/*
mvn test -Dtest=TestPendingReplication > /dev/null & 
mvn_pid=$!

while ! test -f "$logfile"
do
  sleep 1
  echo "Still waiting"
done

sleep 1

testpid=$(jps | grep surefire | awk '{print $1}')
bmsubmit.sh -u
bminstall.sh $testpid

file=$instrumentation_plan_dir/current_b$id.btm
echo $file
bmsubmit.sh $file
bmsubmit.sh
sleep 15
grep 'BM' $logfile > ~/log/log_compare/python/logs/current_b$id.log
echo $logfile

cd -
echo current_b$id.log


