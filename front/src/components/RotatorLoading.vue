<template>
    <v-container>
        <v-layout text-xs-center wrap>
            <v-flex xs12 sm6 offset-sm3>
                <v-progress-circular :size="50" color="primary" indeterminate></v-progress-circular>
            </v-flex>
        </v-layout>
    </v-container>
</template>

<script>
export default {
    name: 'RotatorLoading',
    data() {
        return {
        }
    }
}
</script>
