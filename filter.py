from collections import Counter
import struct
import os.path as op
import json
import pprint


class SetEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, set):
            return list(obj)
        return json.JSONEncoder.default(self, obj)

def filter_index_by_ranges(index_filepath, ranges_filepath, output_filepath):
    """ ranges = "functions.txt"
    index = "index.txt"
    output_file = "filtered_index.json" """
    CWD = op.dirname(__file__)


    ranges_arr = []
    index_arr = {}
    output_dict = {}


    size = 0


    print("working on range sets..")
    with open(op.join(CWD,ranges_filepath), 'r') as f:

        for line in f.readlines():
            num_range = line.split(',')[:3]
            try:
                if int(num_range[0],0):
                    size += int(num_range[2],0)
                    num_range = num_range[:2]
                    nums = list(range(int(num_range[0],0), int(num_range[1],0)))
                    ranges_arr.append(nums)
            except:
                pass


    range_sets = set.union(*map(set,ranges_arr))
    print("range sets finished!")


    print("getting offsets...")
    with open(op.join(CWD,index_filepath), 'r') as f:
        hex_val = -1
        size = 0
        offsets = []
        for line in f.readlines():
            line = line.strip(', \n')
            line = line.split(',')
            if len(line) > 1:
                hex_val = int(line[0])
                size = int(line[1])
            elif line[0] is not ']':
                offsets.append(int(line[0]))
            elif line[0] is ']' and hex_val != -1:
                value = {}
                value["size"] = size
                value["offsets"] = set(offsets)
                index_arr[hex_val] = value
                offsets = []
    print("offsets finished!")


    output_dict = index_arr.copy()


    print("identifying compliant offsets...")
    for key in index_arr.keys():
        output_dict[key]['compliant'] = index_arr[key]['offsets'].intersection(range_sets)
        output_dict[key]['different'] = index_arr[key]['offsets'].difference(range_sets)
    print("compliant offsets finished!")


    print(f'Store in {output_filepath}')
    json.dump(output_dict,open(output_filepath, 'w'), cls=SetEncoder)


    print("finished")


    return output_dict


    
