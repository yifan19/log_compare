id=$1
# port=$2
echo branchid=$id
echo HADOOP_HOME=$HADOOP_HOME
echo COMPARE_HOME$COMPARE_HOME
echo INSTRUMENTATION_HOME$INSTRUMENTATION_HOME

# echo $port
cd $HADOOP_HOME
reportdir=$HADOOP_HOME/hadoop-hdfs-project/hadoop-hdfs/target/surefire-reports/
logfile=$reportdir/org.apache.hadoop.hdfs.server.blockmanagement.TestPendingReplication-output.txt
instrumentation_plan_dir=$COMPARE_HOME/python/plans/
myinstrumentation_jar=$INSTRUMENTATION_HOME/target/uber-blameMasterInstrument-1.0.jar

instrumentation_plans=$(ls $instrumentation_plan_dir/*)
result_dir=$COMPARE_HOME/python/logs/

cd $INSTRUMENTATION_HOME
mvn package -DskipTests
cd -

rm $reportdir/*

file=$instrumentation_plan_dir/current_b$id.btm
echo $file

mvn test -Dtest=TestPendingReplication -DargLine="-javaagent:~/byteman-download-4.0.21/lib/byteman.jar=listener:true,boot:~/byteman-download-4.0.21/lib/byteman.jar,script:$file,port:$port" > /dev/null &
mvn_pid=$!

while ! test -f "$logfile"
do
  sleep 1
  echo "Still waiting"
done

sleep 5

testpid=$(jps | grep surefire | awk '{print $1}')
java -cp $JAVA_HOME/lib/tools.jar:$myinstrumentation_jar \
    ca.uoft.drsg.bminstrument.Launcher $myinstrumentation_jar $testpid

for plan in $instrumentation_plans
do
    java -cp $JAVA_HOME/lib/tools.jar:$myinstrumentation_jar \
        ca.uoft.drsg.bminstrument.PseudoClient "add $plan"
done

wait $mvn_pid

grep 'BM' $logfile > $result_dir/current_b$id.log
awk '/Start Stack Trace/,/End Stack Trace/' $logfile >> $result_dir/current_b$id.log
echo $logfile

# cd -
# echo current_b$id.log


