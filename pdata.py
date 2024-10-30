from collections import Counter
import struct

nms_path = "NMS.exe"

pdata = []
reject = []

with open(nms_path, 'rb') as f:
    data = f.read()
    # First, find the start and ends of the .rdata section.
    pdata_hdr = data.find(b'.pdata')
    pdata_size, _, _, pdata_offset = struct.unpack_from(
        '<IIII',
        data,
        offset=pdata_hdr + 8
    )
    entries =  pdata_size // 0xC
    for i in range(entries):
        func_start, func_end = struct.unpack_from('<II', data, offset=pdata_offset + 0xC * i)
        pdata.append((func_start, func_end, func_end - func_start))

with open("functions.txt", "w") as f:
    f.write("Start,End,Size;\n")
    for func in pdata:
        if func[0] % 8 == 0:
            f.write(f"0x{func[0]:X},0x{func[1]:X},0x{func[2]:X},\n")
        else:
            reject.append(func)

with open("rejects.txt", "w") as f:
    f.write("Start,End,Size;\n")
    for func in reject:
        f.write(f"0x{func[0]:X},0x{func[1]:X},0x{func[2]:X},\n")


# sizes = Counter([x[2] for x in pdata])
# with open("function_size_frequency.txt", "w") as f:
#     f.write("Size\tFrequency\n")
#     for key, value in sizes.most_common():
#         f.write(f"0x{key:X}:\t{value}\n")