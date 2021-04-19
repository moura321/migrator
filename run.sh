benchmark='qsort2020-01-191912.nvt'
#('basicmath2020-03-0218:11.nvt' 'fft2020-01-191758.nvt' 'qsort2020-01-191912.nvt' 'typeset2020-01-191847.nvt')

buffer='256'

counter='4'

ttm='32768'

promote='4'

demote='2'


./migrator input/$benchmark f $buffer $counter $ttm $promote $demote
