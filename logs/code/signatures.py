#!/usr/bin/python3

import re
import json

function_signatures = {}
function_classes = {}
func = 'chooseRandom(int,java.lang.String,java.util.Set,long,int,java.util.List,boolean,org.apache.hadoop.hdfs.StorageType)'
function_signatures['chooseRandom'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.blockmanagement.BlockPlacementPolicyDefault'
func = 'addIfIsGoodTarget(org.apache.hadoop.hdfs.server.blockmanagement.DatanodeStorageInfo,java.util.Set,long,int,boolean,java.util.List,boolean,org.apache.hadoop.hdfs.StorageType)'
function_signatures['addIfIsGoodTarget'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.blockmanagement.BlockPlacementPolicyDefault'
func = 'isGoodTarget(org.apache.hadoop.hdfs.server.blockmanagement.DatanodeStorageInfo,long,int,boolean,java.util.List,boolean,org.apache.hadoop.hdfs.StorageType)'
function_signatures['isGoodTarget'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.blockmanagement.BlockPlacementPolicyDefault'
func = 'chooseLocalStorage(org.apache.hadoop.net.Node,java.util.Set,long,int,java.util.List,boolean,org.apache.hadoop.hdfs.StorageType,boolean)'
function_signatures['chooseLocalStorage'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.blockmanagement.BlockPlacementPolicyDefault'

func = 'rollBlocksScheduled(long)'
function_signatures['rollBlocksScheduled'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.server.blockmanagement.DatanodeDescriptor'
func = '<init>(java.lang.String,java.lang.String,java.lang.String,int,int,int,int,long,long,long,long,long,long,long,int,java.lang.String,org.apache.hadoop.hdfs.protocol.DatanodeInfo$AdminStates)'
function_signatures['DatanodeInfo'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.protocol.DatanodeInfo'
func = 'setRemaining(long)'
function_signatures['setRemaining'] = func
function_classes[func] = 'org.apache.hadoop.hdfs.protocol.DatanodeInfo'

with open('function_signatures.json', 'w') as f:
    json.dump(function_signatures, f)

with open('function_classes.json', 'w') as c:
    json.dump(function_classes, c)
