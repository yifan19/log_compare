reportdir=hadoop-hdfs-project/hadoop-hdfs/target/surefire-reports/
logfile=$reportdir/org.apache.hadoop.hdfs.server.blockmanagement.TestPendingReplication-output.txt
instrumentation_plan_dir=/home/ubuntu/instrumentation_plan/
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
bminstall.sh $testpid

for file in $instrumentation_plan_dir/current_b*.btm
do
  id=$(basename $file | sed 's/^current_b\([0-9]*\).btm$/\1/')
  bmsubmit.sh $file
  bmsubmit.sh
  sleep 5
  grep 'BM' $logfile > ~/logs/current_b$id.log
  echo $id
done
