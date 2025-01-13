// // 在我搞清楚mem到底放了什么东西之前，我先不实现这几个函数了
// int16_t readInt(uint16_t adr) {
//     printf("readInt is called\n");
//     (void)adr;
//     return 0;
//     /* int16_t n; */
//     /* int8_t *nPtr; */
//     /* if(adr < RAM_SIZE - 1){ */
//     /*   nPtr = (int8_t*)&n; */
//     /*   *nPtr = lge_mem[adr++]; */
//     /*   nPtr++; */
//     /*   *nPtr = lge_mem[adr]; */
//     /*   return n; */
//     /* } */
//     /* return 0; */
// }

// uint8_t readMem(uint16_t adr) {
//     printf("readMem is called\n");
//     (void)adr;
//     return 0;
//     /* return (adr < RAM_SIZE) ? lge_mem[adr] : 0; */
// }
