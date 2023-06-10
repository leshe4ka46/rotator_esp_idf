<template>
    <v-container>
        <v-layout text-xs-center wrap>
            <v-flex xs12 sm6 offset-sm3>
                <v-card :color="'#FF0000'" dark>
                    <div class="d-flex flex-no-wrap justify-space-between">
                        <div>
                            <v-card-title class="text-h5">
                                {{ (text.length > 0) ? text : "Произошла ошибка при загрузке страницы" }}
                            </v-card-title>
                        </div>
                    </div>
                </v-card>
            </v-flex>
        </v-layout>
    </v-container>
</template>

<script>
export default {
    props: ["text"],
    name: 'RotatorError',
    data() {
        return {
        }
    }
}
</script>
