namespace Slice_Default_Steps {
        std::vector<float> input = {1.0451847, 0.6245613, 0.48420018, 2.1407738, 1.4023418, 0.6803044, 1.2552906, 0.23355322, 0.5949201, 0.665767, 0.7255075, 0.61573493, -0.22523554, 0.609465, -1.9059293, 1.3349898, -0.4083868, -0.30549952, 1.7568158, -0.09013642, 0.7510219, -0.052165933, 0.045655653, 0.6894599, 0.6663751, -0.13001254, -0.99388146, 1.1250099, 0.90145993, -3.4663842, 1.4765558, -0.8621666, -0.04465484, 0.51125, 0.8123038, -0.13915698, 1.2800659, 1.1580552, 1.8424264, -0.29321876, 0.031092152, 1.0013733, -0.7283629, 0.3382554, -1.8718901, -0.047066037, -1.9088744, 0.42447698, -0.3465933, -0.5751067, 1.5138085, -1.0871423, 0.6077587, 1.776671, 0.34022397, -0.03549139, -1.329498, 0.8371586, -1.4539177, 0.21200769, 0.76399386, -0.9069966, -0.0950934, 1.3569304, -0.29720822, 0.1822998, -1.1559794, 0.04441483, -1.4136242, 0.23844138, 1.0097063, 0.65586853, -2.0127554, -0.2072184, -0.4014769, -0.3000392, 0.8629856, -0.08507111, -1.1835829, -1.0837615, 0.50113016, -1.141945, 1.4552537, -0.36788568, -1.6010555, -0.15414086, -0.7613445, 1.6708724, 1.4167747, -0.096510775, -0.15186574, 0.4412115, -1.2044533, -0.044650316, -0.6854727, -1.4451612, -0.11577169, -1.2647926, -0.14461602, -0.47596788, 0.10738016, 0.06727146, 0.22507443, 1.2529004, -1.89923, 1.447481, 1.1985252, 0.3427861, -0.4630263, -0.65935296, 1.4358126, -2.2243154, 0.49377045, 1.4613236, -0.1912351, 1.8992213, -0.94443643, 0.6515869, 0.31404996, 0.074261054, 0.42852008, -0.19033097, -0.482317, -2.5970418, -1.2141722, 0.74168557, -0.91024095, 0.18518865, 0.20616573, 0.67188245, -1.9931237, 1.6288077, -0.1337348, -1.0095295, -0.292675, -2.8238084, 1.6436734, -0.66659415, 0.31128094, 0.25534976, -0.08932365, -0.24432416, -1.116715, 0.8437299, 1.5952227, -1.2897215, 0.5800979, -0.75728, 0.82642055, -0.20025311, -0.84525776, -0.4629382, 0.09837887, 0.8285737, 0.34570113, -0.4494489, -0.06607181, 1.880768, 0.95183575, 1.6169115, 0.2602527, -1.2811539, -0.12291974, -0.55588114, -2.4634414, -0.29807645, -0.32441446, -0.60882235, -0.6670523, -0.84318537, -1.1632277, -0.16034018, 0.8423004, 1.5689149, -1.378128, -1.9285303, -0.9074993, -0.25396672, -0.90653914, -1.0881042, 0.56759405, -1.4169359, -2.5133574, -0.96242374, -1.8407485, 0.21979266, -1.5453613, -0.400642, 0.3806829, 1.1271498, 1.1819715, 0.847391, -1.4687508, 0.74352884, -0.6923194, -1.4369518, 0.97722024, 1.0514591, -0.90551496, -0.79178625, 0.19880198, 0.6863783, 0.72862554, 1.763804, 0.2279675, 1.4966544, 0.7106558, -0.41115803, -0.21305746, 0.79997617, 0.7268194, 1.5965521, -1.4865237, -0.44435644, -0.36344638, -0.11310481, 1.4190954, 0.6319442, 0.745741, -0.5167261, -0.16796711, 1.5669242, -1.4218544, 0.61177474, -1.243831, -0.408289, 0.865704, -0.97820926, 1.053678, -1.9482862, 0.54905266, -0.9755587, -1.8869104, 0.0044822586, -1.1138756, -0.71788013, 0.7361194, -0.7153275, -1.6451069, 1.7545587, -0.75075376, -1.3882002, -0.5430022, -1.2329432, 0.21638516, -0.326573, 0.5647263, -1.8826942, -0.5760186, 0.51380414, -1.5062225, 0.3432146, 0.14499806, -1.1523787, 2.335982, -2.4507418, 0.5823965, 0.7784748, 1.2795119, -0.39526865, -0.74181443, 1.476051, -0.07681689, 0.0067599732, -0.98287404, -1.3786954, 0.4984134, 1.0468203, -0.7151032, -2.0332944, 0.05419711, -2.3168552, 1.122624, -0.53438026, -2.3934145, -0.49050105, -0.9205437, -0.15586168, 1.6762931, 0.5607007, 0.07048273, -0.7124037, -1.819932, 1.0152652, 1.8492514, 0.09532632, -0.47131425, 0.10136383, 1.6027571, -0.055490978, -0.53033817, 0.47595662, -0.124515995, 0.8248915, -0.50315917, -0.09490088, 2.1328678, 0.26436484, -0.3090177, 0.66534674, -1.2549512, -0.25315812, -1.0608811, 0.08174163, 2.3469543, 0.18253551, 1.4174005, 0.5833562, 0.7734026, 0.8875707, 0.46253937, -0.59789574, 0.3320457, 0.7252614, 0.2282751, 0.6393503, 0.3354368, -2.2898176, 0.08617805, -0.98621976, 1.4499558, -0.10378551, 1.4655052, 0.57472426, -0.8805005, -0.73588294, 0.6668061, 0.81059366, -0.59290576, -0.31642014, 0.5465175, -0.16706206, -0.68133485, 1.0575606, 0.415264, -1.7762902, 0.35459134, -0.036745973, 0.28332543, -0.0061813043, -0.8579742, -1.266533, -0.69849193, -1.4563639, 0.9827122, 0.7408832, -1.514766, -1.0668796, 0.1426218, -0.09352196, -1.4858966, -0.80005354, -0.731151, 0.5344746, 0.38867953, 0.5330557, 0.50632566, 0.6302994, -0.569997, 0.7838313, -0.28514034, -2.2912836, -0.6982049, 0.7933591, -1.1312126, 1.3734623, 1.100901, -0.7409093, 0.38863868, 0.24726652, -0.26152653, -1.6399634, 0.32046428, -0.06809312, 1.6628146, -2.0482807, -0.13053721, -1.700966, 0.3613013, 0.024943631, 0.73767537, 0.35799584, -1.1839654, -0.816691, 1.2237171, -1.9319222, -0.14407313, 1.75666, 1.7709233, 0.21359767, 0.9950233, -0.4615518, 0.44868615, 1.4312615, 1.3808835, -0.18940794, 1.0416954, -0.24796851, 0.48987696, 1.2978021, 0.05604007, -0.3529608, -1.3806872, 2.2423518, 0.38141087, 1.1842633, -0.3420007, -0.28834683, -0.52113175, -0.08655031, 0.66746104, 0.4264375, 0.08894845, 1.361398, -1.1102597, 0.8127054, -0.25339708, 0.7393206, 0.54270566, 0.72499883, 1.0655539, -0.43516365, 1.53332, -1.3317435, 0.9998057, 0.41541678, 0.98901755, -0.7852148, 1.9416982, -0.72119594, -0.034244604, -0.011740846, -0.25054318, -1.4232942, 0.9649547, 0.010748758, 0.13203041, 0.25134632, 1.7783864, 0.0403642, 2.4485521, -0.5401216, -0.3214275, -0.7718592, 1.0362864, -0.7257519, -0.53041863, 0.27436903, 0.08478759, 1.3883367, -0.66223633, 0.51782465, -1.1076697, 0.15442872, 1.6057725, -0.5522208, 0.6561317, 0.17577524, 0.4332249, -0.28804585, -0.5691257, 2.25104, -1.1636895, 0.22430561, 1.0721023, 0.792772, -0.47398332, -0.76994264, 1.0576593, 0.88267803, -1.4782726, 0.8926852, -0.46385735, -0.0716391, -1.665406, 0.11861658, 0.37563065, 0.85860866, -1.8741595, -0.5219604, -1.0582875, 1.7154264, 0.1313024, 1.4848208, 0.11640747, -1.7609112, 0.7769073, 0.48588625, -1.2146038, 0.73792857, -0.9774584, 0.07676499, -1.0037707, 0.015130967, -1.2565023, 1.7358211, 1.4210244, 0.053746346, -1.5248967, 1.1289326, -0.53315526, -1.3237956, -0.21802673, 0.5033034, -0.9388878, -2.9923463, 0.8051657, 0.9151123, 0.017421184, -1.4353907, 1.781005, 0.95736814, 1.1634779, 0.64479274, -0.23042154, -0.55330545, 0.41722617, -0.007924688, 1.9704095, -0.52791095, -0.25249967, 1.217987, -0.26531398, 0.31962937, -1.1676388, 0.17290594, -0.68421125, -0.34032997, -1.4233708, -0.9743128, 0.9177001, -0.14544213, -0.27916044, -1.4160591, 0.096208386, -1.4026262, -0.51816446, 0.9189864, 1.0548416, -0.8247552, -0.7701502, -0.98002684, 1.1631306, 0.8712015, -0.5758766, -1.6995598, 0.09796931, 0.27600157, -0.2175099, -1.2554561, -0.07213151, 2.4231257, 0.7729675, 0.030608136, -1.8162596, 0.7449758, 0.6113035, -0.22608364, -0.6480759, 0.4915602, 1.1545128, 0.35355297, 1.6369073, 0.411864, -1.0840435, -0.075121745, 2.2060554, -0.24247673, -1.1374276, 0.10677125, 1.9537716, -1.108019, -0.32708606, 0.5778413, 1.2812556, 0.21232277, 0.13396217, 0.80973107, 0.26933447, -0.20669563, 1.9351953, 0.7844493, 0.10339695, -1.7368128, 0.823303, 0.67611474, -0.14529933, -0.94407207, 0.6184946, -1.1973552, 0.94807374, -1.9689158, -0.95736796, -0.36089993, 0.97640216, -0.5079678, -0.13345084, -0.09096543, 1.6033889, 0.20670862, -0.27726498, 0.068673365, -0.1626727, 0.13826686, 0.55320334, -1.5936158, -0.82023424, 0.27161613, 0.41867435, -0.9805353, -0.4154204, 0.49877793, 0.0093282405, 0.59771943, 0.09775646, 0.58804435, -0.65016276, 0.73460424, 0.8446652, 2.0283346, 1.2307235, 1.1308593, -0.5006046, 0.334564, -0.97043395, -0.5636059, -1.4977788, 0.87005043, -1.1603388, 0.31635118, -1.8745527, 0.7862731, -1.041438, -0.9785689, -0.014248349, -1.222529, -0.5575574, 2.4933984, -0.064805016, 0.39413083, -1.2988446, 0.7563059, -0.79205245, -1.0542104, 0.5979833, 1.5992491, -1.7063024, -0.5771334, 1.6361665, -0.60626614, 0.44358745, 0.36404964, -0.17300287, -1.4895558, 0.93172455, -0.66854924, 0.4801032, 0.09470631, 0.19432715, -0.39213803, 0.23912194, 1.1788458, -0.20358092, 1.2247528, -1.7846833, 1.7758663, 0.87859154, -0.4107461, -0.32997409, -1.5755576, -1.4550736, -0.31620994, 0.81590563, 0.21436447, -0.0009418542, 0.3039241, -0.14782928, -0.779694, -0.26116168, -0.5875628, 1.064051, 1.520253, 0.68341565, -1.9807853, 0.74206716, -2.8454256, 1.2709564, 1.7051587, -0.23825008, 0.13465987, -0.25501144, 0.13449395, -0.08085852, -0.9345905, -0.9593099, 0.6859107, -1.1454093, -1.3381015, 0.2353102, -0.13322814, 0.11062508, 0.17510203, 0.00516871, -1.0370462, -0.32723942, 1.2823414, -0.042340446, -0.3114818, -8.870333e-05, -0.5615099, -0.20567372, -1.2969033, -0.18617158, 0.58420277, -0.9223417, -0.8050735, -0.36499628, -0.76930875, 1.4448764, -0.1731726, -0.19603184, -0.23324072, -0.109300986, 1.0571263, 1.178747, -0.5669078, -0.6129132, 0.2746348, -0.072358064, 0.3385931, -0.31736866, -0.4328864, 0.9560206, -0.44862372, -0.350604, 0.37914753, 0.76631945, 0.5211334, -1.1975573, -0.39863473, 1.7790956, 0.036023278, 0.85484296, -0.14574684, -2.1517622, 1.4256386, 0.07762644, 0.76923335, -1.452289, -0.44079757, 1.5147146, 0.057785936, 1.7878892, -0.61763746, -0.7521893, 0.7968369, 0.2493956, 0.23879473, 0.047723737, 0.45276004, -0.5033289, 0.84546775, 0.087485075, -0.29491672, -0.7279333, 1.047645, -1.5160652, -0.7238233, -1.4873251, -0.9596354, -0.09308696, 0.9633187, 0.17714372, 0.38465348, -0.13031037, -0.48849452, -0.5216643, -0.7692474, -0.4057824, 0.024406806, 0.33862096, -0.4138596, 0.4039324, 0.18644312, -0.61818093, 0.13876703, -2.164961, -0.31688243, 0.7040236, 0.59295374, -0.8303941, -0.9530634, -0.48112786, -1.3758682, 0.23894647, 0.44065222, -0.7498823, 0.4367827, -0.8626759, -0.819069, 0.6019, 0.40171462, -0.989695, 0.9679922, 0.60419935, 0.5596958, 0.5186218, 0.4689634, -0.28469092, 0.007876219, -0.017619386, 0.65712726, 0.28158224, 0.42113513, -0.5614362, 1.1735151, 0.2318658, -0.60902905, -0.83569163, 1.2109544, -1.7506728, 1.1711022, 0.18666989, -1.4357256, 1.7505751, 2.359068, -0.69469744, -0.17320465, -0.49060386, 1.9384009, -0.8286886, -1.9119048, -0.0867032, 1.675301, -0.15963833, 0.47802424, -0.93921524, 0.21669552, 0.5973385, -0.3897737, -0.96138793, -1.1460986, -0.6993545, 0.48859787, 0.49144292, 0.884664, -0.45526063, 0.06822358, 1.2684873, 1.2708554, 0.08385541, 0.788578, 0.038803414, -0.58519393, 1.4615723, 0.36900017, -0.8346276, 0.04170542, -0.41653165, 1.0975499, 2.0970829, 0.18264593, 1.5489473, 0.5285782, -1.0232004, 0.050813664, -1.0207909, -0.064489454, -1.2171016, -0.4630668, -0.4305054, 1.0489885, -0.3447226, 0.47696728, 0.085851595, 0.45969304, 0.9397857, 0.9925845, 0.84165156, 0.58367836, 0.44830862, 0.79763794, 1.1915636, 0.20407252, -1.1486843, 1.3992747, -0.76383877, 2.411353, 0.48628262, 0.3500077, -0.300855, 0.6324427, 0.34935966, 0.54504216, -1.1055669, -0.09944116, 0.95708686, 1.5737662, -0.5069417, -0.7603594, 0.16570005, 0.38484627, 0.7854822, -0.5289874, -0.20853232, -0.25366986, 0.93255436, 2.1678638, 2.5284173, 0.09104326, -0.25922984, 0.104944386, 0.46791264, -0.1947828, 0.2517247, -0.0018040794, -0.36763376, -2.4569225, 1.2765278, -0.96315217, -1.2554164, -0.2874503, 0.9034364, 0.6889874, -1.4450033, 1.6024436, 0.21677066, -0.008955182, -0.16781831, -1.0223466, -0.96528006, -2.1202044, 0.3416743, 0.021002049, -0.12881091, -1.4332885, -0.12481898, 0.93586546, -1.5900887, -1.0640641, 0.8249464, -0.47171867, 0.1742785, 0.6709329, -1.7259985, 0.78818256, -1.2866964, -1.4600148, -1.4257796, 0.34334445, -2.1116593, 0.25954735, -0.4888973, 0.19879743, -0.27516934, 1.6078179, 1.5662538, 0.14704464, -0.93423027, -0.6393435, -1.1420057, 0.9135534, 0.57406884, 0.1670467, 0.90328616, -1.3994925, -0.31774673, 0.46857116, 0.77330434, 0.45956635, -0.9270845, -1.0549059, -0.5243828, -0.85040414, -1.5719082, 1.0715257, 1.9476706, -1.4326566, 0.97081816, 0.7446318, -0.34977925, -0.9901644, 1.5866337, -0.97141254, -1.1351146, -0.09391565, 0.31912678, -0.13078365, 1.498845, 0.3680228, 0.76907545, 0.37397268, -0.56707764, -0.8323829, 1.3317215, -0.3441633, -2.1597042, -1.7520211, -0.47263923, -0.8540615};
        float output[] = {2.1407738, 0.5949201, 0.609465, 1.7568158, 0.6894599, 0.90145993, 0.51125, 1.8424264, 0.3382554, -0.3465933, 1.776671, -1.4539177, 1.3569304, -1.4136242, -0.2072184, -1.1835829, -0.36788568, 1.4167747, -0.044650316, -0.14461602, 1.2529004, -0.4630263, 1.4613236, 0.31404996, -2.5970418, 0.20616573, -1.0095295, 0.31128094, 0.8437299, 0.82642055, 0.8285737, 0.95183575, -0.55588114, -0.6670523, 1.5689149, -0.90653914, -0.96242374, 0.3806829, 0.74352884, -0.90551496, 1.763804, -0.21305746, -0.44435644, 0.745741, 0.61177474, 1.053678, 0.0044822586, -1.6451069, -1.2329432, -0.5760186, -1.1523787, 1.2795119, 0.0067599732, -0.7151032, -0.53438026, 1.6762931, 1.0152652, 1.6027571, 0.8248915, -0.3090177, 0.08174163, 0.7734026, 0.7252614, 0.08617805, 0.57472426, -0.59290576, 1.0575606, 0.28332543, -1.4563639, 0.1426218, 0.5344746, -0.569997, 0.7933591, 0.38863868, -0.06809312, 0.3613013, -0.816691, 1.7709233, 1.4312615, 0.48987696, 2.2423518, -0.52113175, 1.361398, 0.54270566, -1.3317435, 1.9416982, -1.4232942, 1.7783864, -0.7718592, 0.08478759, 0.15442872, 0.4332249, 0.22430561, 1.0576593, -0.0716391, -1.8741595, 1.4848208, -1.2146038, 0.015130967, -1.5248967, 0.5033034, 0.017421184, 0.64479274, 1.9704095, 0.31962937, -1.4233708, -1.4160591, 1.0548416, 0.8712015, -0.2175099, 0.030608136, -0.6480759, 0.411864, -1.1374276, 0.5778413, 0.26933447, -1.7368128, 0.6184946, -0.36089993, 1.6033889, 0.13826686, 0.41867435, 0.59771943, 0.8446652, 0.334564, -1.1603388, -0.9785689, -0.064805016, -1.0542104, 1.6361665, -1.4895558, 0.19432715, 1.2247528, -0.32997409, 0.21436447, -0.26116168, -1.9807853, -0.23825008, -0.9345905, 0.2353102, -1.0370462, -8.870333e-05, 0.58420277, 1.4448764, 1.0571263, -0.072358064, -0.44862372, -1.1975573, -0.14574684, -1.452289, -0.61763746, 0.047723737, -0.29491672, -1.4873251, 0.38465348, -0.4057824, 0.18644312, 0.7040236, -1.3758682, -0.8626759, 0.9679922, -0.28469092, 0.42113513, -0.83569163, -1.4357256, -0.49060386, 1.675301, 0.5973385, 0.48859787, 1.2684873, -0.58519393, -0.41653165, 0.5285782, -1.2171016, 0.47696728, 0.84165156, 0.20407252, 0.48628262, 0.54504216, -0.5069417, -0.5289874, 2.5284173, -0.1947828, 1.2765278, 0.6889874, -0.16781831, 0.021002049, -1.5900887, 0.6709329, -1.4257796, 0.19879743, -0.93423027, 0.1670467, 0.77330434, -0.85040414, 0.97081816, -0.97141254, 1.498845, -0.8323829, -0.47263923};
}