benchmarks=('basicmath2020-03-0218:11.nvt' 'fft2020-01-191758.nvt' 'qsort2020-01-191912.nvt' 'dijkstra2020-01-191729.nvt')

buffer=('0')
#unlimited

counter=('2048')

ttm=('16' '32' '64' '128' '256' '512')

#promote=('4' '5' '6' '7' '8' '9')

#demote=('6' '5' '4' '3' '2' '1')


for bench in "${benchmarks[@]}"
do
    for buff in "${buffer[@]}"
    do
        for c in "${counter[@]}"
        do      
            for t in "${ttm[@]}"
            do
                ./migrator input/$bench e $buff $c $t >> output/$bench.csv
                ./migrator input/$bench o $buff $c $t >> output/$bench.csv
            done
        done 
    done 
done 

